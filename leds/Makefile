obj-m += led_v1_misc.o
obj-m += led_v2_gpio_leds.o


KERNEL_DIR ?= /home/whqee/MYS-6ULX-20190312-M12/04-Sources/MYiR-iMX-Linux

all:
	make -C $(KERNEL_DIR) \
			ARCH=arm CROSS_COMPILE=arm-poky-linux-gnueabi- \
			SUBDIRS=$(PWD) modules
	cp *.ko /srv/nfs4
	make clean
	@echo "done."

clean:
	make -C $(KERNEL_DIR) \
			ARCH=arm CROSS_COMPILE=arm-poky-linux-gnueabi- \
			SUBDIRS=$(PWD) clean

deploy:
	cp *.ko /srv/nfs4/