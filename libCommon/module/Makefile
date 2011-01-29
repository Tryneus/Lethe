OS_TYPE := $(shell uname -s)
OS_ARCH := $(shell uname -m)

KERNEL_SRC := /usr/src/linux-source-2.6.32
EXTRA_CFLAGS = -I$(PWD)/../include
PWD := $(shell pwd)
obj-m := eventfd.o

all:
	make -C $(KERNEL_SRC) SUBDIRS=$(PWD) modules

clean:
	rm -rf eventfd.ko eventfd.o eventfd.mod.c eventfd.mod.o modules.order Module.symvers

