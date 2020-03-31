#include <linux/module.h>
#include <linux/i2c.h>
#include <linux/of.h>
#include <linux/fs.h>
#include <linux/uaccess.h>
#include <linux/miscdevice.h>

struct pcf8574_dev_data {
    struct i2c_client *client;
    struct miscdevice pcf8574_miscdevice;
    // char name[8];
};


static int pcf8574_read_file(struct file *file, const char __user *userbuf,
                            size_t count, loff_t *ppos)
{
    struct pcf8574_dev_data *data = container_of(file->private_data,
                                                struct pcf8574_dev_data,
                                                pcf8574_miscdevice);
    dev_info(&data->client->dev,
            "read() entered on %s.\n", data->pcf8574_miscdevice.name);

    int ret = i2c_smbus_read_byte(data->client);
    if (ret < 0)
        return -EFAULT;
    
    char buf[3];
    int size = sprintf(buf, "%02x", ret);
    buf[size] = '\n';

    if(*ppos == 0){
		if(copy_to_user(userbuf, buf, size+1)){
			pr_info("Failed to return led_value to user space\n");
			return -EFAULT;
		}
		*ppos+=1;
		return size+1;
	}

	return 0;
}

static int pcf8574_write_file(struct file *file, const char __user *userbuf,
                                size_t count, loff_t *ppos)
{
    struct pcf8574_dev_data *data = container_of(file->private_data,
                                                struct pcf8574_dev_data,
                                                pcf8574_miscdevice);
    dev_info(&data->client->dev, 
        "write() entered on %s.\n", data->pcf8574_miscdevice.name);
    
    dev_info(&data->client->dev, 
        "%zu characters written.\n", count);
    
    char buf[4];
    if(copy_from_user(buf, userbuf, count)) {
		dev_err(&data->client->dev, "Bad copied value\n");
		return -EFAULT;
	}

	buf[count-1] = '\0';

	/* convert the string to an unsigned long */
    unsigned long val;
	int err = kstrtoul(buf, 0, &val);
	if (err)
		return -EINVAL;

    dev_info(&data->client->dev, "the value is %lu\n", val);
    dev_info(&data->client->dev, "the value is %lx\n", val);
    err = i2c_smbus_write_byte(data->client, (uint8_t)val);
    if (err < 0)
        dev_err(&data->client->dev, "the device is not found\n");

    dev_info(&data->client->dev, 
        "write() exited on %s.\n", data->pcf8574_miscdevice.name);

    return count;
}

static int pcf8574_ioctl(struct file *file, uint8_t cmd, uint8_t dat)
{
    struct pcf8574_dev_data *data = container_of(file->private_data,
                                                struct pcf8574_dev_data,
                                                pcf8574_miscdevice);
    // int err;
    if(cmd)
        return i2c_smbus_write_byte(data->client, dat);
    else
        return dat=i2c_smbus_read_byte(data->client);
}

static const struct file_operations pcf8574_fops = {
    .owner = THIS_MODULE,
    .read  = pcf8574_read_file,
    .write = pcf8574_write_file,
    .unlocked_ioctl = pcf8574_ioctl,
};

static int pcf8574_probe(struct i2c_client *client,
            const struct i2c_device_id *id)
{
    struct pcf8574_dev_data * data = 
        devm_kzalloc(&client->dev, sizeof(struct pcf8574_dev_data), GFP_KERNEL);

    i2c_set_clientdata(client,data);
    data->client = client;

    // dev_info(&client->dev, "probe entered on %s.\n", data->name);
    of_property_read_string(client->dev.of_node,"compatible",&data->pcf8574_miscdevice.name);
    dev_info(&client->dev, "probe is entered on %s.\n", data->pcf8574_miscdevice.name);
    // data->pcf8574_miscdevice.name = data->name;
    data->pcf8574_miscdevice.minor = MISC_DYNAMIC_MINOR;
    data->pcf8574_miscdevice.fops = &pcf8574_fops;

    misc_register(&data->pcf8574_miscdevice);
    dev_info(&client->dev, "probe is exited on %s.\n", data->pcf8574_miscdevice.name);
    return 0;
}

static int pcf8574_remove(struct i2c_client *client)
{
    struct pcf8574_dev_data *data =
        i2c_get_clientdata(client);
    dev_info(&client->dev, "remove is entered on %s.\n", data->pcf8574_miscdevice.name);
    misc_deregister(&data->pcf8574_miscdevice);
    dev_info(&client->dev, "remove is exited on %s.\n", data->pcf8574_miscdevice.name);
    return 0;
}

static const struct of_device_id pcf8574_dt_ids[] = {
    { .compatible = "pcf8574", },
    { .compatible = "lcd1602_i2c", },
    { }
};
MODULE_DEVICE_TABLE(of, pcf8574_dt_ids);

static const struct i2c_device_id i2c_ids[] = {
    { .name = "pcf8574", },
    { }
};
MODULE_DEVICE_TABLE(i2c, i2c_ids);

static struct i2c_driver pcf8574_driver = {
    .driver = {
        .name = "pcf8574",
        .owner = THIS_MODULE,
        .of_match_table = pcf8574_dt_ids,
    },
    .probe = pcf8574_probe,
    .remove = pcf8574_remove,
    .id_table = i2c_ids,
};
module_i2c_driver(pcf8574_driver);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("whqee <whqee@foxmail.com>");
MODULE_DESCRIPTION("A i2c IO expanders driver for pcf8574.");

