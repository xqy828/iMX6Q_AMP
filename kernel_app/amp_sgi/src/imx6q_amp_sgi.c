#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/init.h>
#include <linux/uaccess.h>
#include <linux/sched.h>
#include <linux/cpuhotplug.h>
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
#include <linux/irq.h>
#include <linux/interrupt.h>
#include <linux/irqdomain.h>
#include <linux/workqueue.h>

#define DEVICE_NAME "imx6q_amp"
#define DEVICE_NUM 1
#define SGI_MAGIC   'z'
#define CPU3_IPI_ID (15)
#define IRQ_CPU_CORE (0)
#define MAX_SGI 16

struct Sgi_IrqWork
{
    struct workqueue_struct *amp_sgi_wq;
    struct work_struct amp_sgi_work;
};

struct imx6q_amp_cdev
{
    dev_t devid;
    struct cdev cdev;
    struct device *amp_dev;
    struct class *amp_class;
    struct fasync_struct *async_queue;
};

struct imx6q_amp_pdata 
{
    struct device *dev;
    int irq;
    int virq_sgi;
    struct Sgi_IrqWork irq_work;
    struct imx6q_amp_cdev amp_cdev;
};

static DEFINE_PER_CPU(struct imx6q_amp_pdata *, per_cpu_pdata);

static int amp_open(struct inode *inode, struct file *filp)
{
    struct imx6q_amp_pdata *pdata;
	pdata = get_cpu_var(per_cpu_pdata);
	put_cpu_var(per_cpu_pdata);
    filp->private_data = pdata;
    printk(KERN_DEBUG "open amp sgi device success !\n");
    return 0;
}

static long amp_ioctl(struct file *filp, unsigned int cmd, unsigned long arg)
{
    int rc = 0;
    unsigned int this_cpu = 0;
    this_cpu =raw_smp_processor_id();
    printk(KERN_DEBUG "AMP SGI:cpu[%d],cmd:0x%x,Task id:%d,Parent:%s\n",this_cpu,cmd,current->pid,current->comm);
    return rc;
}

static int amp_fasync(int fd, struct file *filp,int mode)
{
    struct imx6q_amp_pdata *pdata;
    pdata = (struct imx6q_amp_pdata*)filp->private_data;

    return fasync_helper(fd,filp,mode,&pdata->amp_cdev.async_queue);    
}

static int amp_release(struct inode *inode, struct file *filp)
{
    struct imx6q_amp_pdata *pdata;
    pdata = (struct imx6q_amp_pdata*)filp->private_data;
    amp_fasync(-1,filp,0);
    printk(KERN_DEBUG "close amp device success !\n");
    return 0;
}

static irqreturn_t cpu3_sgi_kick_handle(int irq, void *data)
{
    struct imx6q_amp_pdata **pdata_ptr = data;
    struct imx6q_amp_pdata *pdata = *pdata_ptr;
    int cpu = smp_processor_id();
    if (!pdata) 
    {
        pr_err_ratelimited("SGI: pdata is NULL on CPU%d\n", cpu);
        return IRQ_NONE;
    }
    dev_info(pdata->dev, "virq sgi %d kick cpu:%d\n",pdata->virq_sgi,cpu);
    queue_work(pdata->irq_work.amp_sgi_wq,&pdata->irq_work.amp_sgi_work);
    return IRQ_HANDLED;
}

static void cpu3_sgi_kick_work_fn(struct work_struct *work)
{
    struct Sgi_IrqWork *irq_work = container_of(work, struct Sgi_IrqWork, amp_sgi_work);
    struct imx6q_amp_pdata *pdata = container_of(irq_work, struct imx6q_amp_pdata, irq_work);
    dev_info(pdata->dev, "work executed on CPU %d\n", smp_processor_id());
    if (pdata->amp_cdev.async_queue) 
    {
        kill_fasync(&pdata->amp_cdev.async_queue, SIGIO, POLL_IN);
        dev_info(pdata->dev, "sent SIGIO\n");
    }
}

static const struct file_operations amp_fops=
{
    .owner = THIS_MODULE,
    .open = amp_open,
    .release = amp_release,
    .fasync = amp_fasync,
    .unlocked_ioctl = amp_ioctl,
};

static int imx6q_amp_cpuhp_start(unsigned int cpu)
{
	struct imx6q_amp_pdata *pdata;

	pdata = get_cpu_var(per_cpu_pdata);
	put_cpu_var(per_cpu_pdata);
	enable_percpu_irq(pdata->virq_sgi, IRQ_TYPE_NONE);
	return 0;
}

static int imx6q_amp_cpuhp_down(unsigned int cpu)
{
	struct imx6q_amp_pdata *pdata;

	pdata = get_cpu_var(per_cpu_pdata);
	put_cpu_var(per_cpu_pdata);
	disable_percpu_irq(pdata->virq_sgi);
	return 0;
}

static void imx6q_disable_percpu_irq(void *data)
{
	struct imx6q_amp_pdata *pdata;

	pdata = *this_cpu_ptr(&per_cpu_pdata);

	disable_percpu_irq(pdata->virq_sgi);
}

static void imx6q_cleanup_sgi(struct imx6q_amp_pdata *pdata)
{
	cpuhp_remove_state(CPUHP_AP_ONLINE_DYN);
	on_each_cpu(imx6q_disable_percpu_irq, NULL, 1);
	irq_clear_status_flags(pdata->virq_sgi, IRQ_PER_CPU);
	free_percpu_irq(pdata->virq_sgi, &per_cpu_pdata);
	irq_dispose_mapping(pdata->virq_sgi);
}

/************************************************//*
Refer to the xlnx_mbox_init_sgi function in the
/drivers/mailbox/zynqmp-ipi-mailbox.c file.   
*************************************************/
static int imx6q_amp_init_sgi(struct platform_device *pdev,int sgi_num,struct imx6q_amp_pdata *pdata)
{
    int ret = 0;
    int cpu = 0;
    struct irq_domain *domain = NULL;
    struct irq_fwspec sgi_fwspec;
    struct device_node *interrupt_parent = NULL;
    struct device *dev = &pdev->dev;
 	
    /* Find GIC controller to map SGIs. */
    interrupt_parent = of_irq_find_parent(dev->of_node);
    if (!interrupt_parent) 
    {
    	dev_err(&pdev->dev, "Failed to find property for Interrupt parent\n");
    	return -EINVAL;
    }
    
    /* Each SGI needs to be associated with GIC's IRQ domain. */
    domain = irq_find_host(interrupt_parent);
    of_node_put(interrupt_parent);
    
    /* Each mapping needs GIC domain when finding IRQ mapping. */
    sgi_fwspec.fwnode = domain->fwnode;
    /*
    * When irq domain looks at mapping each arg is as follows:
    * 3 args for: interrupt type (SGI), interrupt # (set later), type
    */
    sgi_fwspec.param_count = 1;
    /* Set SGI's hwirq */
    sgi_fwspec.param[0] = sgi_num;
    pdata->virq_sgi = irq_create_fwspec_mapping(&sgi_fwspec);
    for_each_possible_cpu(cpu)
        per_cpu(per_cpu_pdata, cpu) = pdata;
    ret = request_percpu_irq(pdata->virq_sgi, cpu3_sgi_kick_handle, pdev->name,&per_cpu_pdata);
    WARN_ON(ret);
    if (ret) 
    {
        irq_dispose_mapping(pdata->virq_sgi);
        return ret;
    }
    irq_set_status_flags(pdata->virq_sgi, IRQ_PER_CPU);
    /* Setup function for the CPU hot-plug cases */
    cpuhp_setup_state(CPUHP_AP_ONLINE_DYN, "amp/sgi:starting",imx6q_amp_cpuhp_start, imx6q_amp_cpuhp_down);
    return ret;
}

static int imx6q_amp_init_cdev(struct platform_device *pdev,struct imx6q_amp_pdata *pdata)
{
    struct imx6q_amp_cdev *pcdev = &pdata->amp_cdev;
    int ret = 0;
    ret = alloc_chrdev_region(&pcdev->devid,0, DEVICE_NUM, DEVICE_NAME);
    if(ret !=0)
    {
        dev_err(&pdev->dev, "unable to allocate device num,ret:%d\n",ret);
        return ret;
    }
    pcdev->cdev.owner = THIS_MODULE;
    cdev_init(&pcdev->cdev,&amp_fops);
    ret = cdev_add(&pcdev->cdev,pcdev->devid,1);
    if(ret != 0)
    {
        dev_err(&pdev->dev, "unable add cdev,ret:%d\n",ret);
        return -EINVAL;
        goto err_del_cdev_region;
    }
    pcdev->amp_class = class_create(DEVICE_NAME);
    if(IS_ERR(pcdev->amp_class))
    {
        dev_err(&pdev->dev,"unable create amp class \n");
        return -EINVAL;
        goto err_del_cdev;
    }
    
    pcdev->amp_dev = device_create(pcdev->amp_class,NULL,pcdev->devid,NULL,DEVICE_NAME);
    if(IS_ERR(pcdev->amp_dev))
    {
        dev_err(&pdev->dev,"unable create amp device \n");
        return -EINVAL;
        goto err_del_class;
    }
    return 0;

err_del_class:
    class_destroy(pcdev->amp_class);
err_del_cdev:
    cdev_del(&pcdev->cdev);
err_del_cdev_region:
    unregister_chrdev_region(pcdev->devid,DEVICE_NUM);
    return ret;
}

static int imx6q_amp_init_irq_wrok(struct platform_device *pdev,struct imx6q_amp_pdata *pdata)
{
    int ret = 0;
    struct Sgi_IrqWork *work = &pdata->irq_work;

    INIT_WORK(&work->amp_sgi_work, cpu3_sgi_kick_work_fn);
    work->amp_sgi_wq = create_workqueue("cpu3_sgi_kick");
    if(!work->amp_sgi_wq)
    {		
        ret = -EINVAL;
        dev_err(&pdev->dev, "failed create cpu3_sgi_kick workqueue,ret:%d\n",ret);
        return ret;
    }
    return 0;
}

static int imx6q_amp_probe(struct platform_device *pdev)
{
    int ret = 0;
    int irq = 0;
    struct device *dev = &pdev->dev;
    //struct of_phandle_args out_irq;
    struct imx6q_amp_pdata __percpu *pdata;
    struct device_node *np = pdev->dev.of_node;
    int cpu = smp_processor_id();
    dev_info(dev, "current cpu %d\n",cpu);
    pdata = devm_kzalloc(dev,sizeof(struct imx6q_amp_pdata),GFP_KERNEL);
    if (!pdata)
        return -ENOMEM;
    pdata->dev = dev;
#if 0
    ret = of_irq_parse_one(dev_of_node(dev), 0, &out_irq);
    if (ret < 0) 
    {
    	dev_err(dev, "failed to parse interrupts\n");
    	return -1;
    }
    irq = out_irq.args[1];
#endif
	ret = of_property_read_u32(np, "nxp,sgi-intid", &irq);
    if (ret < 0) 
    {
        dev_err(dev, "No sgi intid is specified.\n");
        return ret;
    }
    if(irq < MAX_SGI)
    {
        pdata->irq = irq;
        ret = imx6q_amp_init_sgi(pdev,pdata->irq,pdata);
        if(ret)
        {
            dev_err(dev, "failed to init sgi.\n");
            ret = -EINVAL;
            return ret;
        }
    }
    else
    {
        irq =  platform_get_irq(pdev, 0);
        if(irq < 0)
        {            
            dev_err(dev, "failed to get irq.\n");
            ret = -EINVAL;
            return ret;
        }
        pdata->irq = irq;
        ret = devm_request_irq(dev, pdata->irq, cpu3_sgi_kick_handle,IRQF_SHARED, dev_name(dev), pdata);
    }
    if (ret) 
    {
        dev_err(dev, "IRQ %d is not requested successfully.\n",pdata->irq);
        ret = -EINVAL;
        return ret;
    }
    dev_info(dev, "HW IRQ %d,VIRQ %d, requested successfully.\n",pdata->irq,pdata->virq_sgi);
    ret = imx6q_amp_init_cdev(pdev,pdata);
    if(ret)
    {
        dev_err(dev, "cdev create failed.\n");
        ret = -EINVAL;
        goto err_free_irq;
    }
    ret = imx6q_amp_init_irq_wrok(pdev,pdata);
    if(ret)
    {
        dev_err(dev, "irq wrok create failed.\n");
        ret = -EINVAL;
        goto err_free_cdev;
    }
    platform_set_drvdata(pdev, pdata);
    return 0;
err_free_cdev:
    device_unregister(pdata->amp_cdev.amp_dev);
    class_destroy(pdata->amp_cdev.amp_class);
    cdev_del(&pdata->amp_cdev.cdev);
    unregister_chrdev_region(pdata->amp_cdev.devid,DEVICE_NUM);
err_free_irq:
    if(irq < MAX_SGI)
    {
        imx6q_cleanup_sgi(pdata);
    }
    return ret;
}

static void imx6q_amp_remove(struct platform_device *pdev)
{
	struct imx6q_amp_pdata *pdata;
	pdata = platform_get_drvdata(pdev);
    device_unregister(pdata->amp_cdev.amp_dev);
    class_destroy(pdata->amp_cdev.amp_class);
    cdev_del(&pdata->amp_cdev.cdev);
    unregister_chrdev_region(pdata->amp_cdev.devid,DEVICE_NUM);
    destroy_workqueue(pdata->irq_work.amp_sgi_wq);
    if(pdata->irq < MAX_SGI)
    {
        imx6q_cleanup_sgi(pdata);
    }
}

static const struct of_device_id imx6q_amp_of_match[] = {
	{ 
        .compatible = "nxp,imx6q-amp",
 	},
 	{},
 };
 MODULE_DEVICE_TABLE(of, imx6q_amp_of_match);
 
 static struct platform_driver imx6q_amp_driver = {
 	.probe = imx6q_amp_probe,
 	.remove_new = imx6q_amp_remove,
 	.driver = {
 		   .name = "imx6q-amp",
 		   .of_match_table = of_match_ptr(imx6q_amp_of_match),
 	},
 };

static int __init imx6q_Amp_Init(void)
{
	return platform_driver_register(&imx6q_amp_driver);
}

static void __exit imx6q_Amp_Exit(void)
{
	platform_driver_unregister(&imx6q_amp_driver);
}

module_init(imx6q_Amp_Init); // insmod
module_exit(imx6q_Amp_Exit); // rmmod

MODULE_LICENSE("GPL");
