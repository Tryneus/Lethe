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

clean:
	cd libCommon; make clean
	cd libThreadComm; make clean
	cd libProcessComm; make clean
	cd libSocketComm; make clean
	cd libThreadUtil; make clean
