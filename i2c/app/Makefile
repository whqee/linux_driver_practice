objs = 
objs += oled_app



all: app deploy

# Make does not offer a recursive wildcard function, so here's one:
rwildcard=$(wildcard $1$2) $(foreach d,$(wildcard $1*),$(call rwildcard,$d/,$2))

# How to recursively find all files with the same name in a given folder
ALL_MAIN_C := $(call rwildcard,,main.c)
app:
 : %.c
	$(CC) -o $@ $^

clean:
	rm lcd1602_i2c

deploy:
	cp lcd1602_i2c /srv/nfs4/ -rf