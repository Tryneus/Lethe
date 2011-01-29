all:
	cd libCommon; make all
	cd libThreadComm; make all
	cd libProcessComm; make all
	cd libSocketComm; make all
	cd libThreadUtil; make all

install:
	cd libCommon; make install

runTest:
	cd libCommon; make runTest
	cd libThreadComm; make runTest
	cd libProcessComm; make runTest
	cd libSocketComm; make runTest
	cd libThreadUtil; make runTest

# install is used to install the kernel module needed for mutex/semaphore/event auto-reset on linux
install:
	cd libCommon; make all

clean:
	cd libCommon; make clean
	cd libThreadComm; make clean
	cd libProcessComm; make clean
	cd libSocketComm; make clean
	cd libThreadUtil; make clean
