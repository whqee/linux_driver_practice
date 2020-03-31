#include <linux/module.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/platform_device.h>
#include <linux/gpio/consumer.h>
#include <linux/interrupt.h>
#include <linux/of.h>
#include <linux/of_gpio.h>


static struct gpio_desc *led0, *led1, *btn1;
static unsigned int led01, irq;
static int flag = 0;

static irq_handler_t btn1_pushed_irq_handler(unsigned int irq,
                        void *dev_id, struct pt_regs *regs)
{
    int state;
    state = gpiod_get_value(btn1);
    flag = ~flag;
    gpiod_set_value(led0, flag);
    gpiod_set_value(led1, flag);
    pr_info("btn1 interrupt: Interrupt! state is %d\n", state);
    return IRQ_HANDLED;
}


static int my_drv_probe(struct platform_device *pdev)
{
    struct device *dev = &pdev->dev;
    led0 = gpiod_get_index(dev, "led", 0, GPIOD_OUT_LOW);
    if (IS_ERR(led0)) {
        dev_err(dev, "led0 get failed.\n");
        return PTR_ERR(led0);
    }

    led1 = gpiod_get_index(dev, "led", 1, GPIOD_OUT_LOW);
    if (IS_ERR(led1)) {
        dev_err(dev, "led1 get failed.\n");
        return PTR_ERR(led1);
    }

    // struct device_node *np = &pdev->dev.of_node;
    // if (!np)
    //     return ERR_PTR(-ENOENT);
    // led1 = of_get_named_gpio(np, "led", 1);
    // gpio_request(led1, "led1");

    btn1 = devm_gpiod_get(dev, "btn1", GPIOD_IN);
    if (IS_ERR(btn1)) {
        dev_err(dev, "btn1 get failed.\n");
        return PTR_ERR(btn1);
    }
    gpiod_set_debounce(btn1, 200);

    irq = gpiod_to_irq(btn1);
    if (irq < 0) {
        dev_err(dev, "irq is not available\n");
        return -EINVAL;
    }
    int retval;
    retval = request_threaded_irq(irq, NULL,
                        btn1_pushed_irq_handler,
                        IRQF_TRIGGER_LOW | IRQF_ONESHOT,
                        "gpio-descriptor-sample",
                        NULL);
    if (retval) {
        dev_err(dev, "Failed to request interrupt %d, error %d\n", irq, retval);
        return retval;
    }
    pr_info("Hello gpiod probed !\n");
    return 0;
}

static int my_drv_remove(struct platform_device *pdev)
{
    free_irq(irq,NULL);
    gpiod_put(led0);
    gpiod_put(led1);
    gpiod_put(btn1);
    pr_info("removed!\n");
    return 0;
}

static const struct of_device_id gpiod_dt_ids[] = {
    { .compatible = "gpio-descriptor-sample", },
    { }
};

static struct platform_driver mypdrv = {
    .probe = my_drv_probe,
    .remove = my_drv_remove,
    .driver = {
        .name = "gpio-descriptor-sample",
        .of_match_table = gpiod_dt_ids,
        .owner = THIS_MODULE,
    },
};

module_platform_driver(mypdrv);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("whqee <whqee@foxmail.com>");
MODULE_DESCRIPTION("led V2");