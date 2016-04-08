# pass kernel version via command line like this:
# example: make KERNELVER=4.2.0-35-generic USERNAME=os
# where my kernel version is *0-35 and my username is os.
obj-m += character_device_driver.o

all:
	make -C /lib/modules/$(KERNELVER)/build M=/home/$(USERNAME)/hw5 modules
