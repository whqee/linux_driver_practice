#include <linux/module.h>
#include <linux/fs.h>
#include <linux/spi/spi.h>
#include <linux/of_gpio.h>
#include <linux/spi/spi_gpio.h>
#include <linux/input.h>
#include <linux/platform_device.h>


static int ds1302_probe(struct spi_device *spi)
{
    pr_info("probed.\n");
    struct rtc_device *rtc;
    u8 addr;
    u8 buf[4];
    u8 *bp;
    int status;

    if (spi->bits_per_word && (spi->bits_per_word != 8)) {
		dev_err(&spi->dev, "bad word length\n");
		return -EINVAL;
	} else if (spi->max_speed_hz > 2000000) {
		dev_err(&spi->dev, "speed is too high\n");
		return -EINVAL;
	} else if (spi->mode & SPI_CPHA) {
		dev_err(&spi->dev, "bad mode\n");
		return -EINVAL;
	}

    addr = 0x81;
	status = spi_write_then_read(spi, &addr, sizeof(addr), buf, 1);
	if (status < 0) {
		dev_err(&spi->dev, "control register read error %d\n",
				status);
		return status;
	}
    pr_info("sec: %d\n", buf[0]);


    return 0;
}

static int ds1302_remove(struct spi_device *spi)
{
	//spi_set_drvdata(spi, NULL);
    dev_info(&spi->dev, "removed\n");
	return 0;
}

static const struct of_device_id ds1302_dt_ids[] = {
    { .compatible = "ds1302", },
    {}
};
MODULE_DEVICE_TABLE(of, ds1302_dt_ids);

static const struct spi_device_id ds1302_id[] = {
    { .name = "ds1302", },
    {}
};
MODULE_DEVICE_TABLE(spi, ds1302_id);

static struct spi_driver ds1302_driver = {
    .driver = {
        .name = "ds1302",
        .owner = THIS_MODULE,
        .of_match_table = ds1302_dt_ids,
    },
    .probe = ds1302_probe,
    .remove = ds1302_remove,
    .id_table = ds1302_id,
};
module_spi_driver(ds1302_driver);


MODULE_LICENSE("GPL");
MODULE_AUTHOR("whqee <whqee@foxmail.com>");
MODULE_DESCRIPTION("SPI");
