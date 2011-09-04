OS_TYPE := $(shell uname -s)
OS_ARCH := $(shell uname -m)

KERNEL_SRC := /lib/modules/`uname -r`/build
obj-m := eventfd-lethe.o timerfd-lethe.o

all: eventfd-lethe.c timerfd-lethe.c eventfd-internal.h timerfd-internal.h
	make -C $(KERNEL_SRC) M=`pwd` modules

install: all
	insmod eventfd-lethe.ko
	mknod /dev/eventfd-lethe c 245 0
	chmod a+wr /dev/eventfd-lethe
	insmod timerfd-lethe.ko
	mknod /dev/timerfd-lethe c 246 0
	chmod a+wr /dev/timerfd-lethe

uninstall:
	rmmod eventfd-lethe
	rm /dev/eventfd-lethe
	rmmod timerfd-lethe
	rm /dev/timerfd-lethe

clean:
	rm -rf eventfd-lethe.ko eventfd-lethe.o eventfd-lethe.mod.c eventfd-lethe.mod.o modules.order Module.symvers .eventfd-lethe.ko.cmd .eventfd-lethe.mod.o.cmd .eventfd-lethe.o.cmd .tmp_versions timerfd-lethe.ko timerfd-lethe.o timerfd-lethe.mod.c timerfd-lethe.mod.o .timerfd-lethe.ko.cmd .timerfd-lethe.mod.o.cmd .timerfd-lethe.o.cmd

