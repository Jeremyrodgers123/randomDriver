obj-m := myRand.o
KDIR := /lib/modules/$(shell uname -r)/build
PWD := $(shell pwd)

all:
	$(MAKE) -C $(KDIR) M=$(PWD) modules

clean:
	$(MAKE) -C $(KDIR) M=$(PWD) clean

load:
	sudo insmod myRand.ko
unload:
	sudo rmmod myRand.ko
p:
	dmesg | tail
test:
	python3 testDriver.py
reset: unload clean all load test

r: unload clean all load test	
	dmesg
