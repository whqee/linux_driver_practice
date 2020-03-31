obj-m += rtc-ds1302.o
obj-m += rtc-ds1302s.o
obj-m += spi-test.o


KERNEL_DIR ?= /home/whqee/MYS-6ULX-20190312-M12/04-Sources/MYiR-iMX-Linux

all: modules deploy

modules:
	@make -C $(KERNEL_DIR) \
			ARCH=arm CROSS_COMPILE=arm-poky-linux-gnueabi- \
			SUBDIRS=$(PWD) modules

clean:
	@make -C $(KERNEL_DIR) \
			ARCH=arm CROSS_COMPILE=arm-poky-linux-gnueabi- \
			SUBDIRS=$(PWD) clean

deploy:
	cp *.ko /srv/nfs4/ -rf
	make clean
	@echo "done."