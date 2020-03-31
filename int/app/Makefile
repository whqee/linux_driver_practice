all: gpio_keys

app : gpio_keys.c
	$(CC) -o $@ $^
	-cp -f gpio_keys /srv/nfs4/
	
clean :
	rm gpio_keys
