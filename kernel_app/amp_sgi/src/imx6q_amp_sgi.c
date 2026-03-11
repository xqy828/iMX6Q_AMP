#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/init.h>
#include <linux/uaccess.h>
#include <linux/sched.h>
#include <asm/current.h>
#include <asm/irq.h>
#include <asm/io.h>
#include <linux/device.h>
#include <linux/cdev.h>
#include <linux/of_address.h>
#include <linux/of_device.h>
#include <linux/of_platform.h>
#include <linux/of_irq.h>
#include <linux/fcntl.h>  
#include <linux/device.h>
#include <linux/irqchip/arm-gic.h>

#define DEVICE_NAME "amp_sgi"
#define DEVICE_NUM 1

#define SGI_MAGIC   'z'
#define TRIGGER_CPU1  (_IO(SGI_MAGIC,0))

typedef struct
{
    unsigned int soft_cpu3_intid;
}ampsgi_param;


typedef struct
{
    dev_t devid;
    struct cdev cdev;
    struct class *class;
    struct device *device;
    ampsgi_param param;
    struct fasync_struct *async_queue;
}stru_ampsgi;

static stru_ampsgi gstAmpSgiStru;

static int AmpSgi_open(struct inode *inode, struct file *filp)
{
    printk(KERN_DEBUG "open amp sgi device success !\n");
    return 0;
}

static long AmpSgi_ioctl(struct file *filp, unsigned int cmd, unsigned long arg)
{
    int rc = 0;
    unsigned int this_cpu = 0;
    this_cpu =raw_smp_processor_id();
    printk(KERN_DEBUG "AMP SGI:cpu[%d],cmd:0x%x,Task id:%d,Parent:%s\n",this_cpu,cmd,current->pid,current->comm);
    switch(cmd)
    {
        case TRIGGER_CPU1 : 
            gic_raise_softirq(cpumask_of(1),gstAmpSgiStru.param.soft_cpu1_intid);
            printk(KERN_DEBUG "AMP SGI Kick Cpu[%d],SGI[%d]\n",1,gstAmpSgiStru.param.soft_cpu1_intid);
            break;
        default :
            rc =  -EINVAL;
            break;
    }
    return rc;
}

static int AmpSgi_fasync(int fd, struct file *filp,int mode)
{
    return fasync_helper(fd,filp,mode,&gstAmpSgiStru.async_queue);    
}

static int AmpSgi_release(struct inode *inode, struct file *filp)
{
    AmpSgi_fasync(-1,filp,0);
    printk(KERN_DEBUG "close amp sgi device success !\n");
    return 0;
}


static void cpu1_sgi_kick_handle(void)
{
    if(gstAmpSgiStru.async_queue != NULL)
    {
        printk("sgi %d kick cpu0\n",gstAmpSgiStru.param.soft_cpu1_intid);
        kill_fasync(&gstAmpSgiStru.async_queue,SIGIO,POLL_IN);
    }
    else
    {
        printk("sgi %d kick but async_queue is null \n",gstAmpSgiStru.param.soft_cpu1_intid);
    }
}


static const struct file_operations AmpSgi_fops=
{
    .owner = THIS_MODULE,
    .open = AmpSgi_open,
    .release = AmpSgi_release,
    .fasync = AmpSgi_fasync,
    .unlocked_ioctl = AmpSgi_ioctl,
};

static int AmpSgi_probe(struct platform_device *pdev)
{
    struct device_node *pAmpSgi_node = NULL;
    int rc = 0;
    int elems_count = 0;
    printk(KERN_DEBUG "amp sgi tree probe  . . . \n");

    pAmpSgi_node = pdev->dev.of_node;
    // default SGI ID value 
    gstAmpSgiStru.param.soft_cpu0_intid = 14;
    gstAmpSgiStru.param.soft_cpu1_intid = 15;

    elems_count = of_property_count_elems_of_size(pAmpSgi_node, "cpu0_int_id", sizeof(u32));

    if(elems_count != 1)
    {
        dev_err(&pdev->dev,"get cpu0_int_id error \n");
        return -1;
    }

    rc = of_property_read_u32(pAmpSgi_node,"cpu0_int_id",&(gstAmpSgiStru.param.soft_cpu0_intid));
    if(rc < 0)
    {
        dev_err(&pdev->dev,"get cpu0_int_id value error \n");
    }

    elems_count = of_property_count_elems_of_size(pAmpSgi_node, "cpu1_int_id", sizeof(u32));

    if(elems_count != 1)
    {
        dev_err(&pdev->dev,"get cpu1_int_id error \n");
        return -1;
    }

    rc = of_property_read_u32(pAmpSgi_node,"cpu1_int_id",&(gstAmpSgiStru.param.soft_cpu1_intid));
    if(rc < 0)
    {
        dev_err(&pdev->dev,"get cpu1_int_id value error \n");
    }

    printk(KERN_DEBUG "amp sgi cpu0_sgi_intid=%d,cpu1_sgi_intid=%d. \n",gstAmpSgiStru.param.soft_cpu0_intid,gstAmpSgiStru.param.soft_cpu1_intid);

    rc = set_ipi_handler(gstAmpSgiStru.param.soft_cpu0_intid,cpu1_sgi_kick_handle,"cpu1 kick cpu0");
    if(rc != 0)
    {
        dev_err(&pdev->dev,"cpu0 SGI %d handler already registered \n",gstAmpSgiStru.param.soft_cpu0_intid);
    }

    printk(KERN_DEBUG "amp sgi cpu0_sgi_intid=%d,register. \n",gstAmpSgiStru.param.soft_cpu0_intid);

    rc = alloc_chrdev_region(&gstAmpSgiStru.devid,0, DEVICE_NUM, DEVICE_NAME);
    if(rc !=0)
    {
        dev_err(&pdev->dev,"unable to allocate device num \n");
    }
    
    gstAmpSgiStru.cdev.owner = THIS_MODULE;
    cdev_init(&gstAmpSgiStru.cdev,&AmpSgi_fops);
    rc = cdev_add(&gstAmpSgiStru.cdev,gstAmpSgiStru.devid,1);
    if(rc !=0)
    {
        dev_err(&pdev->dev,"unable add cdev \n");
    }

    gstAmpSgiStru.class = class_create(THIS_MODULE,DEVICE_NAME);
    if(IS_ERR(gstAmpSgiStru.class))
    {
        dev_err(&pdev->dev,"unable create amp class \n");
    }
    
    gstAmpSgiStru.device = device_create(gstAmpSgiStru.class,&pdev->dev,gstAmpSgiStru.devid,NULL,DEVICE_NAME);
    if(IS_ERR(gstAmpSgiStru.device))
    {
        dev_err(&pdev->dev,"unable create amp device \n");
    }
    return 0;
}

static int AmpSgi_remove(struct platform_device *pdev)
{
    device_unregister(gstAmpSgiStru.device);
    class_destroy(gstAmpSgiStru.class);
    cdev_del(&gstAmpSgiStru.cdev);
    unregister_chrdev_region(gstAmpSgiStru.devid,DEVICE_NUM);
    clear_ipi_handler(gstAmpSgiStru.param.soft_cpu0_intid);
    return 0;
}


static struct of_device_id ampsgi_of_match[] = 
{
    {.compatible = "ampsgi",},
    {/* end of list */},
};

MODULE_DEVICE_TABLE(of,ampsgi_of_match);

static struct platform_driver ampsgi_driver = 
{
    .driver = 
    {
        .name = DEVICE_NAME,
        .owner = THIS_MODULE,
        .of_match_table = ampsgi_of_match,
    },
    .probe = AmpSgi_probe,
    .remove = AmpSgi_remove,
};

static int __init AmpSgiInit(void)
{
    int ret = 0;
    printk(KERN_DEBUG "amp sgi module init ... !\n");
    ret  = platform_driver_register(&ampsgi_driver);
    return ret;
}

static void __exit AmpSgiExit(void)
{
    platform_driver_unregister(&ampsgi_driver);
    printk(KERN_ALERT "amp sgi module exit !\n");
}


module_init(AmpSgiInit); // insmod
module_exit(AmpSgiExit); // rmmod

MODULE_LICENSE("GPL");


