#include <linux/module.h>
#include <linux/spi/spi.h>
#include <linux/input.h>
#include <linux/interrupt.h>
#include <linux/of_gpio.h>

#define MAX_SPI_FREQ_HZ		5000000
#define MAX_FREQ_NO_FIFODELAY	1500000
#define ADXL345_CMD_MULTB	(1 << 6)
#define ADXL345_CMD_READ	(1 << 7)
#define ADXL345_WRITECMD(reg)	(reg & 0x3F)
#define ADXL345_READCMD(reg)	(ADXL345_CMD_READ | (reg & 0x3F))
#define ADXL345_READMB_CMD(reg) (ADXL345_CMD_READ | ADXL345_CMD_MULTB \
					| (reg & 0x3F))



struct adxl345_bus_ops {
    u16 bustype;
    int (*read)(struct device *, unsigned char);
    int (*read_block)(struct device *, unsigned char, int, void *);
    int (*write)(struct device *, unsigned char, unsigned char);
};

struct adxl345_priv {
    struct device *dev;
    struct input_dev *input;
    const struct adxl345_bus_ops *bops;
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
                                                        int count, void *buf)
{
    struct spi_device *spi = to_spi_device(dev);
    ssize_t status;

    reg = 0x80 | 0x40 | (reg & 0x3f);
    status = spi_write_then_read(spi, &reg, 1, buf, count);

    return (status < 0) ? status : 0;
}

static const struct adxl345_bus_ops adxl345_spi_bops = {
    .bustype    = BUS_SPI,
    .write      = adxl345_spi_write,
    .read       = adxl345_spi_read,
    .read_block = adxl345_spi_read_block,
};

// static struct adxl345_priv *adxl345_probe()
// {

// }

static int adxl345_spi_probe(struct spi_device *spi)
{
    struct adxl345_priv *ac_priv;
    struct input_dev *input_dev;
    int err;

    ac_priv = devm_kzalloc(spi->dev, sizeof(*ac_priv), GFP_KERNEL);
    input_dev = input_allocate_device();

    if (!ac_priv || !input_dev) {
        devm_kfree(ac_priv);
        input_free_device(input_dev);
        return -ENOMEM;
    }
    



}



MODULE_LICENSE("GPL");
MODULE_AUTHOR("whqee <whqee@foxmail.com>");
MODULE_DESCRIPTION("ADXL345 Three-Axis Digital Accelerometer SPI Bus Driver");
