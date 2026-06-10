#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/init.h>
#include <linux/uaccess.h>
#include <linux/device.h>
#include <linux/cdev.h>
#include <linux/of_device.h>
#include <linux/of_platform.h>
#include <linux/platform_device.h>
#include <linux/gpio/consumer.h>
#include <linux/delay.h>
#include <linux/reboot.h>
#include <asm/barrier.h>

#define DEVICE_NAME  "imx6q_reboot"
#define DEVICE_NUM   1
#define REBOOT_MAGIC 'R'
#define REBOOT_IOC_RESET _IO(REBOOT_MAGIC, 0x01)

struct reboot_pdata {
    struct device *dev;
    struct gpio_desc *reset_gpio;
    dev_t devid;
    struct cdev cdev;
    struct class *class;
    struct device *chrdev;
};

static int reboot_open(struct inode *inode, struct file *filp)
{
    struct reboot_pdata *pdata = container_of(inode->i_cdev, struct reboot_pdata, cdev);
    filp->private_data = pdata;
    dev_dbg(pdata->dev, "reboot device opened\n");
    return 0;
}

static long reboot_ioctl(struct file *filp, unsigned int cmd, unsigned long arg)
{
    struct reboot_pdata *pdata = filp->private_data;

    switch (cmd) {
    case REBOOT_IOC_RESET:
        dev_info(pdata->dev, "RESET: syncing barrier before GPIO assert\n");
        /*
         * DSB SY ensures all prior memory accesses (Normal memory writes,
         * MMIO stores, cache maintenance ops) drain from the store buffer
         * before the GPIO Device-memory write below. Without it, a Normal
         * memory store could still be in the store buffer when the reset
         * fires, causing data loss.
         *
         * Full dcache flush (flush_cache_all + outer_flush_all) would be
         * needed to push dirty L1/L2 lines to DDR, but those symbols are
         * not exported to modules. The userspace reboot app should call
         * sync() before sending this ioctl to flush filesystem data.
         */
        wmb(); /* compiler + DMB(ST) barrier for prior stores */
        dsb(sy); /* drain store buffer, fence ARMv7 weakly-ordered memory */
        gpiod_set_value(pdata->reset_gpio, 1); /* GPIO_ACTIVE_LOW: 1 = assert (low) */
        break;
    default:
        dev_dbg(pdata->dev, "unknown ioctl cmd 0x%x\n", cmd);
        return -ENOTTY;
    }
    return 0;
}

static int reboot_release(struct inode *inode, struct file *filp)
{
    struct reboot_pdata *pdata = filp->private_data;
    dev_dbg(pdata->dev, "reboot device closed\n");
    return 0;
}

static const struct file_operations reboot_fops = {
    .owner          = THIS_MODULE,
    .open           = reboot_open,
    .release        = reboot_release,
    .unlocked_ioctl = reboot_ioctl,
};

static int reboot_probe(struct platform_device *pdev)
{
    int ret;
    struct device *dev = &pdev->dev;
    struct reboot_pdata *pdata;

    pdata = devm_kzalloc(dev, sizeof(*pdata), GFP_KERNEL);
    if (!pdata)
        return -ENOMEM;
    pdata->dev = dev;

    pdata->reset_gpio = devm_gpiod_get(dev, "reset", GPIOD_OUT_HIGH);
    if (IS_ERR(pdata->reset_gpio)) {
        ret = PTR_ERR(pdata->reset_gpio);
        if (ret == -EPROBE_DEFER)
            dev_info(dev, "GPIO deferred, retry later\n");
        else
            dev_err(dev, "failed to get reset GPIO: %d\n", ret);
        return ret;
    }

    ret = alloc_chrdev_region(&pdata->devid, 0, DEVICE_NUM, DEVICE_NAME);
    if (ret) {
        dev_err(dev, "failed to allocate chrdev region: %d\n", ret);
        return ret;
    }

    cdev_init(&pdata->cdev, &reboot_fops);
    pdata->cdev.owner = THIS_MODULE;
    ret = cdev_add(&pdata->cdev, pdata->devid, 1);
    if (ret) {
        dev_err(dev, "failed to add cdev: %d\n", ret);
        goto err_unregister_region;
    }

    pdata->class = class_create(DEVICE_NAME);
    if (IS_ERR(pdata->class)) {
        ret = PTR_ERR(pdata->class);
        dev_err(dev, "failed to create class: %d\n", ret);
        goto err_del_cdev;
    }

    pdata->chrdev = device_create(pdata->class, NULL, pdata->devid, NULL, DEVICE_NAME);
    if (IS_ERR(pdata->chrdev)) {
        ret = PTR_ERR(pdata->chrdev);
        dev_err(dev, "failed to create device: %d\n", ret);
        goto err_destroy_class;
    }

    platform_set_drvdata(pdev, pdata);
    dev_info(dev, "reboot driver probed, /dev/%s created\n", DEVICE_NAME);
    return 0;

err_destroy_class:
    class_destroy(pdata->class);
err_del_cdev:
    cdev_del(&pdata->cdev);
err_unregister_region:
    unregister_chrdev_region(pdata->devid, DEVICE_NUM);
    return ret;
}

static void reboot_remove(struct platform_device *pdev)
{
    struct reboot_pdata *pdata = platform_get_drvdata(pdev);

    device_unregister(pdata->chrdev);
    class_destroy(pdata->class);
    cdev_del(&pdata->cdev);
    unregister_chrdev_region(pdata->devid, DEVICE_NUM);
    dev_info(pdata->dev, "reboot driver removed\n");
}

static const struct of_device_id reboot_of_match[] = {
    { .compatible = "nxp,imx6q-reboot" },
    { },
};
MODULE_DEVICE_TABLE(of, reboot_of_match);

static struct platform_driver reboot_driver = {
    .probe    = reboot_probe,
    .remove_new = reboot_remove,
    .driver   = {
        .name           = DEVICE_NAME,
        .of_match_table = of_match_ptr(reboot_of_match),
    },
};

static int __init reboot_init(void)
{
    return platform_driver_register(&reboot_driver);
}

static void __exit reboot_exit(void)
{
    platform_driver_unregister(&reboot_driver);
}

module_init(reboot_init);
module_exit(reboot_exit);

MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("i.MX6Q Reboot Driver via GPIO6_31");
