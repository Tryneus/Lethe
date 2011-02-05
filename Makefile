
all: prepare
	cd libCommon; make all
	cd libThreadComm; make all
	cd libProcessComm; make all
	cd libSocketComm; make all
	cd libThreadUtil; make all

prepare:
	cd thirdParty; make prepare

# install is used to install the kernel module needed for mutex/semaphore/event auto-reset on linux
install: all
	cd libCommon; make install

runTest: all
	cd libCommon; make runTest
	cd libThreadComm; make runTest
	cd libProcessComm; make runTest
	cd libSocketComm; make runTest
	cd libThreadUtil; make runTest

valTest: all
	cd libCommon; make valTest
	cd libThreadComm; make valTest
	cd libProcessComm; make valTest
	cd libSocketComm; make valTest
	cd libThreadUtil; make valTest

check: prepare
	cppcheck --quiet --enable=all \
                 -IlibCommon/include \
                 -IlibThreadComm/include \
                 -IlibProcessComm/include \
                 -IlibSocketComm/include \
                 -IlibThreadUtil/include \
                 -IthirdParty/mct/include \
                 libCommon/src \
                 libThreadComm/src \
                 libProcessComm/src \
                 libSocketComm/src \
                 libThreadUtil/test \
          2>&1 | tee check.log

checkTest: prepare
	cppcheck --quiet --enable=all \
                 -IlibCommon/include \
                 -IlibThreadComm/include \
                 -IlibProcessComm/include \
                 -IlibSocketComm/include \
                 -IlibThreadUtil/include \
                 -IthirdParty/mct/include \
                 -IthirdParty/catch/include \
                 libCommon/test \
                 libThreadComm/test \
                 libProcessComm/test \
                 libSocketComm/test \
                 libThreadUtil/test \
          2>&1 | tee checkTest.log

clean:
	cd thirdParty; make clean
	cd libCommon; make clean
	cd libThreadComm; make clean
	cd libProcessComm; make clean
	cd libSocketComm; make clean
	cd libThreadUtil; make clean
