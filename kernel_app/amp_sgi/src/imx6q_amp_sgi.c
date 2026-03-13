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
#include <linux/interrupt.h>

static DEFINE_PER_CPU(int, sgi_percpu_data);
#define DEVICE_NAME "amp_sgi"
#define DEVICE_NUM 1
#define SGI_MAGIC   'z'

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
    struct workqueue_struct *sgi_wq;
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

static irqreturn_t cpu3_sgi_kick_handle(int irq, void *dev_id)
{
    if(gstAmpSgiStru.async_queue != NULL)
    {
        printk("sgi %d kick cpu0\n",gstAmpSgiStru.param.soft_cpu3_intid);
        kill_fasync(&gstAmpSgiStru.async_queue,SIGIO,POLL_IN);
    }
    else
    {
        printk("sgi %d kick but async_queue is null \n",gstAmpSgiStru.param.soft_cpu3_intid);
    }
    return IRQ_HANDLED;
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
    int rc = 0;
    printk(KERN_DEBUG "amp sgi tree probe  . . . \n");

    // default SGI ID value 
    gstAmpSgiStru.param.soft_cpu3_intid = 15;
    printk(KERN_DEBUG "amp sgi cpu3_sgi_intid=%d \n",gstAmpSgiStru.param.soft_cpu3_intid);

    rc = request_percpu_irq(gstAmpSgiStru.param.soft_cpu3_intid, cpu3_sgi_kick_handle,"cpu3_kick_cpu0", &sgi_percpu_data);
    if(rc != 0)
    {
        dev_err(&pdev->dev,"failed request IRQ %d \n",gstAmpSgiStru.param.soft_cpu3_intid);
    }

    printk(KERN_DEBUG "amp sgi cpu3_sgi_intid=%d,register. \n",gstAmpSgiStru.param.soft_cpu3_intid);

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

    gstAmpSgiStru.class = class_create(DEVICE_NAME);
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
    free_percpu_irq(gstAmpSgiStru.param.soft_cpu3_intid, &sgi_percpu_data);
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


