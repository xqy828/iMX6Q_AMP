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
#include <linux/workqueue.h>

#define DEVICE_NAME "amp_sgi"
#define DEVICE_NUM 1
#define SGI_MAGIC   'z'
#define CPU3_IPI_NUM (15)

typedef struct
{
    unsigned int soft_cpu3_intid;
    struct workqueue_struct *amp_sgi_wq;
    struct work_struct amp_sgi_work;
}Sgi_IrqCFg;

typedef struct
{
    dev_t devid;
    struct cdev cdev;
    struct class *class;
    struct device *device;
    Sgi_IrqCFg irq;
    struct fasync_struct *async_queue;
}Sgi;

static Sgi gstAmpSgi;
Sgi_IrqCFg __percpu *AmpSgi_Irq;


static int AmpSgi_open(struct inode *inode, struct file *filp)
{
    filp->private_data = &gstAmpSgi;
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
    return fasync_helper(fd,filp,mode,&gstAmpSgi.async_queue);    
}

static int AmpSgi_release(struct inode *inode, struct file *filp)
{
    AmpSgi_fasync(-1,filp,0);
    printk(KERN_DEBUG "close amp sgi device success !\n");
    return 0;
}

static irqreturn_t cpu3_sgi_kick_handle(int irq, void *dev_id)
{
    Sgi_IrqCFg *AmpSgi_Irq = dev_id;
    printk("sgi %d kick cpu0\n",gstAmpSgi.irq.soft_cpu3_intid);
    queue_work(AmpSgi_Irq->amp_sgi_wq,&AmpSgi_Irq->amp_sgi_work);
    return IRQ_HANDLED;
}

static void cpu3_sgi_kick_work_fn(struct work_struct *work)
{
    kill_fasync(&gstAmpSgi.async_queue,SIGIO,POLL_IN);
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
    gstAmpSgi.irq.soft_cpu3_intid = CPU3_IPI_NUM;
    printk(KERN_DEBUG "amp sgi cpu3_sgi_intid=%d \n",gstAmpSgi.irq.soft_cpu3_intid);
    AmpSgi_Irq = &gstAmpSgi.irq;
    rc = request_percpu_irq(gstAmpSgi.irq.soft_cpu3_intid, cpu3_sgi_kick_handle,"cpu3_kick_cpu0", AmpSgi_Irq);
    if(rc != 0)
    {
        dev_err(&pdev->dev,"failed request IRQ %d \n",gstAmpSgi.irq.soft_cpu3_intid);
    }

    printk(KERN_DEBUG "amp sgi cpu3_sgi_intid=%d,register. \n",gstAmpSgi.irq.soft_cpu3_intid);

	INIT_WORK(&AmpSgi_Irq->amp_sgi_work, cpu3_sgi_kick_work_fn);
    AmpSgi_Irq->amp_sgi_wq = create_workqueue("cpu3_sgi_kick");

    rc = alloc_chrdev_region(&gstAmpSgi.devid,0, DEVICE_NUM, DEVICE_NAME);
    if(rc !=0)
    {
        dev_err(&pdev->dev,"unable to allocate device num \n");
    }
    
    gstAmpSgi.cdev.owner = THIS_MODULE;
    cdev_init(&gstAmpSgi.cdev,&AmpSgi_fops);
    rc = cdev_add(&gstAmpSgi.cdev,gstAmpSgi.devid,1);
    if(rc !=0)
    {
        dev_err(&pdev->dev,"unable add cdev \n");
    }

    gstAmpSgi.class = class_create(DEVICE_NAME);
    if(IS_ERR(gstAmpSgi.class))
    {
        dev_err(&pdev->dev,"unable create amp class \n");
    }
    
    gstAmpSgi.device = device_create(gstAmpSgi.class,&pdev->dev,gstAmpSgi.devid,NULL,DEVICE_NAME);
    if(IS_ERR(gstAmpSgi.device))
    {
        dev_err(&pdev->dev,"unable create amp device \n");
    }
    return 0;
}

static int AmpSgi_remove(struct platform_device *pdev)
{
    device_unregister(gstAmpSgi.device);
    class_destroy(gstAmpSgi.class);
    cdev_del(&gstAmpSgi.cdev);
    unregister_chrdev_region(gstAmpSgi.devid,DEVICE_NUM);
    free_percpu_irq(gstAmpSgi.irq.soft_cpu3_intid, AmpSgi_Irq);
    destroy_workqueue(AmpSgi_Irq->amp_sgi_wq); 
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


