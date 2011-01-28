all:
	cd libCommon; make all
	cd libThreadComm; make all
	cd libSocketComm; make all
	cd libProcessComm; make all
	cd libThreadUtil; make all
	cd ThreadCommTest; make all

# install is used to install the kernel module needed for mutex/semaphore/event auto-reset on linux
install:
	cd libCommon; make all

clean:
	cd libCommon; make clean
	cd libThreadComm; make clean
	cd libSocketComm; make clean
	cd libProcessComm; make clean
	cd libThreadUtil; make clean
	cd ThreadCommTest; make clean
