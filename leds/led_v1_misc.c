#include <linux/module.h>
#include <linux/fs.h> /* struct file_operations */
#include <linux/platform_device.h> /* platform_driver_register(), platform_set_drvdata() */
#include <linux/io.h>         /* devm_ioremap(), iowrite32() */
#include <linux/of.h>         /* of_property_read_string() */
#include <linux/uaccess.h>    /* copy_from_user(), copy_to_user() */
#include <linux/miscdevice.h> /* misc_register() */

// private led device structure
struct led_dev {
    struct miscdevice led_misc_device;
    u32 led_mask;
    const char *led_name;
    char led_value[8];
};

#define GPDR 0x020AC000    /* GPIO5 DATA register: DR */
#define GPDIR 0x020AC004    /* GPIO5 GPIO direction register: GDIR */
#define LED0_MASK 1 << 1    /* MASK DATA bit, GPIO_LOW to light leds */
#define LED1_MASK 1 << 2
#define ALL_MASK (1 << 1 | 1 << 2)

static void __iomem *GPDR_V;    /* get virtual address */
static void __iomem *GPDIR_V;

static int led_write(struct file *file, const char __user *buff,
                                    size_t count, loff_t *ppos)
{
    u32 read, write;
    struct led_dev *led;
    pr_info("Entering led_write()...\n");

    /* The private_data is led_misc_device structure in the private led_dev structure. */
    led = container_of(file->private_data, struct led_dev, led_misc_device);

    if(copy_from_user(led->led_value, buff, count)) {
        pr_info("Bad copied value!\n");
        return -EFAULT;
    }
    led->led_value[count-1] = '\0';
    pr_info("buff from user space: %s\n", led->led_value);
    
    read = ioread32(GPDR_V);
    if(!strcmp(led->led_value,"1") || !strcmp(led->led_value, "on"))
        iowrite32(read & ~(led->led_mask), GPDR_V);
    else if (!strcmp(led->led_value, "0") || !strcmp(led->led_value, "off"))
        iowrite32(read | (led->led_mask), GPDR_V);
    else {
        pr_info("Bad value!\n");
        return -EINVAL;
    }
    
    pr_info("leaving led_write()...\n");
    return count;
}

static const struct file_operations led_fops = {
    .owner = THIS_MODULE,
    .write = led_write,
};

static int led_probe(struct platform_device *pdev)
{
    pr_info("led_probe entered.\n");

    struct led_dev *led;
    pr_info("%s\n", pdev->name);
    GPDR_V = devm_ioremap(&pdev->dev, GPDR, sizeof(u32));
    GPDIR_V = devm_ioremap(&pdev->dev, GPDIR, sizeof(u32));
    pr_info("0x%08lx\n", GPDR_V);
    pr_info("0x%08lx\n", GPDIR_V);

    iowrite32(ALL_MASK, GPDR_V);
    iowrite32(ALL_MASK, GPDIR_V);

    led = devm_kzalloc(&pdev->dev, sizeof(struct led_dev), GFP_KERNEL);

    of_property_read_string(pdev->dev.of_node, "label", &led->led_name);

    led->led_misc_device.minor = MISC_DYNAMIC_MINOR;
    led->led_misc_device.name = led->led_name;
    led->led_misc_device.fops = &led_fops;

    if (strcmp(led->led_name,"led0") == 0)
        led->led_mask = LED0_MASK;
    else if (strcmp(led->led_name, "led1") == 0)
        led->led_mask = LED1_MASK;
    else {
        pr_info("Bad device tree value...\n");
        return -EINVAL;
    }

    int err = misc_register(&led->led_misc_device);
    if (err)
        return err;
    
    platform_set_drvdata(pdev, led);
    pr_info("led_probe exit!\n");
    return 0;
}


static int led_remove(struct platform_device *pdev)
{
    struct led_dev *led = platform_get_drvdata(pdev);
    iowrite32(ALL_MASK, GPDR_V);
    misc_deregister(&led->led_misc_device);
    pr_info("led_removed!\n");
    return 0;
}

static const struct of_device_id my_of_ids[] = {
    { .compatible = "leds" },
    { .compatible = "gpio-leds" },
    {},
};
MODULE_DEVICE_TABLE(of, my_of_ids);

static struct platform_driver led_platform_driver = {
    .probe = led_probe,
    .remove = led_remove,
    .driver = {
        .name = "gpio-leds",
        .of_match_table = my_of_ids,
        .owner = THIS_MODULE,
    }
};

module_platform_driver(led_platform_driver);
// static int led_init(void)
// {
//     pr_info("led initing...\n");
//     int err = platform_driver_register(&led_platform_driver);
//     if (err) {
//         pr_err("platform value returned %d\n", err);
//         return err;
//     }
//     pr_info("led init done!\n");
//     return 0;
// }
// static void led_exit(void)
// {
//     iowrite32(ALL_MASK, GPDR_V);
//     platform_driver_unregister(&led_platform_driver);
//     pr_info("led driver exit! \n");
// }
// module_init(led_init);
// module_exit(led_exit);


MODULE_LICENSE("GPL");
MODULE_AUTHOR("whqee <whqee@foxmail.com>");
MODULE_DESCRIPTION("A simple miscellaneous led driver module.");
