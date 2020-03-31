#include <linux/module.h>
#include <linux/i2c.h>
#include <linux/platform_device.h>
#include <linux/fs.h>
#include <linux/miscdevice.h>


struct ssd1306_data {
    struct i2c_client *client;
    struct miscdevice misc_dev;
};

static int ssd1306_write_cmd(struct i2c_client *client, uint8_t cmd)
{
    return i2c_smbus_write_byte_data(client, 0x0, cmd);
}

static int ssd1306_write_data(struct i2c_client *client, uint8_t data)
{
    return i2c_smbus_write_byte_data(client, 0x40, data);
}

/*
* cmd: 0 - write data, 1 - write cmd
* value: value to be write
*/
static int ssd1306_ioctl(struct file *file, uint8_t cmd, uint8_t value)
{
    struct ssd1306_data *data = container_of(file->private_data,
                                                struct ssd1306_data,
                                                misc_dev);
    if(cmd)
        return ssd1306_write_cmd(data->client, value);
    else
        return ssd1306_write_data(data->client, value);
}


static const struct file_operations ssd1306_fops = {
    .owner = THIS_MODULE,
    .unlocked_ioctl = ssd1306_ioctl,
};

static int ssd1306_probe(struct i2c_client *client,
                        const struct i2c_device_id *id)
{
    struct ssd1306_data *data;
    data = devm_kzalloc(&client->dev, sizeof(struct ssd1306_data), GFP_KERNEL);

    i2c_set_clientdata(client, data);
    data->client = client;
    data->misc_dev.minor = MISC_DYNAMIC_MINOR;
    data->misc_dev.name = "oled_ssd1306";
    data->misc_dev.fops = &ssd1306_fops;

    misc_register(&data->misc_dev);
    dev_info(&client->dev, "ssd1306 probed.\n");
    return 0;
}

static int ssd1306_remove(struct i2c_client *client)
{
    struct ssd1306_data *data = i2c_get_clientdata(client);
    misc_deregister(&data->misc_dev);
    dev_info(&client->dev, "ssd1306 removed.\n");
    return 0;
}

static const struct of_device_id ssd1306_dt_ids[] = {
    { .compatible = "ssd1306", },
    {},
};
MODULE_DEVICE_TABLE(of,ssd1306_dt_ids);

static const struct i2c_device_id i2c_ids[] = {
    { .name = "ssd1306", },
    { }
};
MODULE_DEVICE_TABLE(i2c, i2c_ids);

static struct i2c_driver ssd1306_driver = {
    .driver = {
        .name = "ssd1306",
        .owner = THIS_MODULE,
        .of_match_table = ssd1306_dt_ids,
    },
    .probe = ssd1306_probe,
    .remove = ssd1306_remove,
    .id_table = i2c_ids,
};
module_i2c_driver(ssd1306_driver);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("whqee <whqee@foxmail.com>");
MODULE_DESCRIPTION("I2C client driver for oled ssd1306.");
