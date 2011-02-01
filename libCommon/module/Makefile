OS_TYPE := $(shell uname -s)
OS_ARCH := $(shell uname -m)

KERNEL_SRC := /lib/modules/`uname -r`/build
obj-m := eventfd.o

all:
	make -C $(KERNEL_SRC) M=`pwd` modules

clean:
	rm -rf eventfd.ko eventfd.o eventfd.mod.c eventfd.mod.o modules.order Module.symvers

