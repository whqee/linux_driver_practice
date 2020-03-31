#include <linux/module.h>
#include <linux/platform_device.h>
#include <linux/interrupt.h>
#include <linux/gpio/consumer.h>
#include <linux/miscdevice.h>

static char *HELLO_KEYS_NAME = "PB_KEY";

static irqreturn_t hello_keys_isr(int irq, void *data)
{
    struct device *dev = data;
    dev_info(dev, "interrupt received. key: %s\n", HELLO_KEYS_NAME);
    return IRQ_HANDLED;
}

static struct miscdevice helloworld_miscdevice = {
    .minor = MISC_DYNAMIC_MINOR,
    .name = "mydev",
};


static int my_probe(struct platform_device *pdev)
{
    int ret, irq;
    struct gpio_desc *gpio;
    struct device *dev = &pdev->dev;

    dev_info(dev, "my_probe() is called.\n");

    /* First method to get the virtual linux IRQ number */
    gpio = devm_gpiod_get(dev, NULL, GPIOD_IN);
    if (IS_ERR(gpio)) {
        dev_err(dev, "gpio get failed.\n");
        return PTR_ERR(gpio);
    }
    irq = gpiod_to_irq(gpio);
    if (irq < 0)
        return irq;
    dev_info(dev, "The IRQ number is: %d\n", irq);

    /* Second method to get the virtual linux IRQ number */
    irq = platform_get_irq(pdev, 0);
    if (irq < 0) {
        dev_err(dev, "irq is not available\n");
        return -EINVAL;
    }
    dev_info(dev, "IRQ_using_platform_get_irq: %d\n", irq);

    /* Allocate the interrupt line */
    ret = devm_request_irq(dev, irq, hello_keys_isr,
                        IRQF_TRIGGER_FALLING,
                        HELLO_KEYS_NAME, dev);
    if (ret) {
        dev_err(dev, "Failed to request interrupt %d, error %d\n", irq, ret);
        return ret;
    }
    
    ret = misc_register(&helloworld_miscdevice);
    if (ret != 0) {
        dev_err(dev, "could not register the misc device mydev\n");
        return ret;
    }
    
    dev_info(dev, "mydev: got minor %i\n", helloworld_miscdevice.minor);
    dev_info(dev, "myprobe() is exited.\n");

    return 0;
}

static int my_remove(struct platform_device *pdev)
{
    dev_info(&pdev->dev, "my_remove() entered\n");
    misc_deregister(&helloworld_miscdevice);
    dev_info(&pdev->dev, "my_remove() exited\n");
    return 0;
}

static const struct of_device_id my_dt_ids[] = {
    { .compatible = "intkey", },
    // { .compatible = "gpio-keys", },
    {},
};
MODULE_DEVICE_TABLE(of, my_dt_ids);

static struct platform_driver my_driver = {
    .probe = my_probe,
    .remove = my_remove,
    .driver = {
        .name = "intkey",
        .of_match_table = my_dt_ids,
        .owner = THIS_MODULE,
    }
};
module_platform_driver(my_driver);


MODULE_LICENSE("GPL");
MODULE_AUTHOR("whqee <whqee@foxmail.com>");
MODULE_DESCRIPTION("");
