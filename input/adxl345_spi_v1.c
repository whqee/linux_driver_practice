#include <linux/module.h>
#include <linux/spi/spi.h>
#include <linux/input.h>
#include <linux/interrupt.h>
#include <linux/of_gpio.h>
#include <linux/delay.h>

#define MAX_SPI_FREQ_HZ		5000000
#define MAX_FREQ_NO_FIFODELAY	1500000
#define ADXL345_CMD_MULTB	(1 << 6)
#define ADXL345_CMD_READ	BIT(7)
#define ADXL345_WRITECMD(reg)	(reg & 0x3F)
#define ADXL345_READCMD(reg)	(ADXL345_CMD_READ | (reg & 0x3F))
#define ADXL345_READMB_CMD(reg) (ADXL345_CMD_READ | ADXL345_CMD_MULTB \
					| (reg & 0x3F))




struct adxl345_priv {
    struct device *dev;
    struct input_dev *input;
};


static int adxl345_spi_read(struct device *dev, unsigned char reg)
{
    struct spi_device *spi = to_spi_device(dev);
    unsigned char cmd;

    cmd = 0x80 | (reg & 0x3f);

    return spi_w8r8(spi, cmd);
}

static int adxl345_spi_write(struct device *dev, unsigned char reg,
                                                    unsigned char val)
{
    struct spi_device *spi = to_spi_device(dev);
    unsigned char buf[2];

    buf[0] = reg & 0x3f;
    buf[1] = val;

    return spi_write(spi, buf, sizeof(buf));
}

static int adxl345_spi_read_block(struct device *dev, unsigned char reg,
                                                        void *buf, int count)
{
    struct spi_device *spi = to_spi_device(dev);
    ssize_t status;

    reg = 0x80 | 0x40 | (reg & 0x3f);
    status = spi_write_then_read(spi, &reg, 1, buf, count);

    return (status < 0) ? status : 0;
}


static int adxl345_spi_probe(struct spi_device *spi)
{
    struct adxl345_priv *ac_priv;
    // struct input_dev *input_dev;
    int err;

    ac_priv = devm_kzalloc(&spi->dev, sizeof(*ac_priv), GFP_KERNEL);
    // input_dev = input_allocate_device();

    if (!ac_priv) {
        err = -ENOMEM;
        dev_err(&spi->dev, "Failed to allocate mem\n");
        goto err_free_mem;
    }

    ac_priv->dev = &spi->dev;
    // ac_priv->input = input_dev;
    
    adxl345_spi_write(ac_priv->dev,0x31,0x0B);   //测量范围,正负16g，13位模式
    adxl345_spi_write(ac_priv->dev,0x2C,0x08);   //速率设定为12.5 参考pdf13页
    adxl345_spi_write(ac_priv->dev,0x2D,0x08);   //选择电源模式   参考pdf24页
    adxl345_spi_write(ac_priv->dev,0x2E,0x80);   //使能 DATA_READY 中断
    adxl345_spi_write(ac_priv->dev,0x1E,0x00);   //X 偏移量 根据测试传感器的状态写入pdf29页
    adxl345_spi_write(ac_priv->dev,0x1F,0x00);   //Y 偏移量 根据测试传感器的状态写入pdf29页
    adxl345_spi_write(ac_priv->dev,0x20,0x05);   //Z 偏移量 根据测试传感器的状态写入pdf29页

    unsigned char devid = adxl345_spi_read(&spi->dev, 0x00);

    if (devid != 0xe5) {
        dev_err(&spi->dev, "Failed to probe adxl345\n");
        err = -ENODEV;
        goto err_free_mem;
    } else {
        dev_err(&spi->dev, "adxl345 probed!\n");
    }

    int i = 100;
    unsigned char buf[6];
    int tmp;
    while(i--) {
        adxl345_spi_read_block(&spi->dev, 0x32, buf, 6);

        tmp = (buf[1]<<8) + buf[0];
        // tmp = (float)tmp*3.9;
        dev_info(ac_priv->dev, "X:0x%x\n", tmp);

        tmp = (buf[3]<<8) + buf[2];
        // tmp = (float)tmp*3.9;
        dev_info(ac_priv->dev, "Y:0x%x\n", tmp);

        tmp = (buf[5]<<8) + buf[4];
        // tmp = (float)tmp*3.9;
        dev_info(ac_priv->dev, "Z:0x%x\n", tmp);

        msleep(1000);
    }
    
    return 0;

err_free_mem:
    // input_free_device(input_dev);
    devm_kfree(ac_priv->dev ,ac_priv);
// err_out:
    return err;
}

static int adxl345_remove(struct spi_device * spi)
{
    dev_info(&spi->dev, "removed!");
    // struct adxl345_priv *ac = spi_get_drvdata(spi);
    // input_unregister_device(ac->input);
    return 0;
}

static const struct of_device_id the_dt_ids[] = {
    { .compatible = "adxl345-spi" },
    { },
};
MODULE_DEVICE_TABLE(of, the_dt_ids);

static const struct spi_device_id adxl345_id[] = {
    { .name = "adxl345", },
    { },
};
MODULE_DEVICE_TABLE(spi, adxl345_id);


static struct spi_driver adxl345_driver = {
    .driver = {
        .name = "adxl345",
        .owner = THIS_MODULE,
        .of_match_table = the_dt_ids,
    },
    .probe = adxl345_spi_probe,
    .remove = adxl345_remove,
    .id_table = adxl345_id,
};
module_spi_driver(adxl345_driver);



MODULE_LICENSE("GPL");
MODULE_AUTHOR("whqee <whqee@foxmail.com>");
MODULE_DESCRIPTION("ADXL345 Three-Axis Digital Accelerometer SPI Bus Driver");
