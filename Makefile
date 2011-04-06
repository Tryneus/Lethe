
all: prepare
	cd LetheCommon; make all
	cd LetheThreadComm; make all
	cd LetheProcessComm; make all
	cd LetheSocketComm; make all
	cd LetheThreadUtil; make all

prepare:
	cd thirdParty; make prepare

# install is used to install the kernel module needed for mutex/semaphore/event auto-reset on linux
install: all
	cd LetheCommon; make install

runTest: all
	cd LetheCommon; make runTest
	cd LetheThreadComm; make runTest
	cd LetheProcessComm; make runTest
	cd LetheSocketComm; make runTest
	cd LetheThreadUtil; make runTest

valTest: all
	cd LetheCommon; make valTest
	cd LetheThreadComm; make valTest
	cd LetheProcessComm; make valTest
	cd LetheSocketComm; make valTest
	cd LetheThreadUtil; make valTest

check: prepare
	cppcheck --quiet --enable=all \
                 -ILetheCommon/include \
                 -ILetheThreadComm/include \
                 -ILetheProcessComm/include \
                 -ILetheSocketComm/include \
                 -ILetheThreadUtil/include \
                 -IthirdParty/mct/include \
                 LetheCommon/src \
                 LetheThreadComm/src \
                 LetheProcessComm/src \
                 LetheSocketComm/src \
                 LetheThreadUtil/test \
          2>&1 | tee check.log

checkTest: prepare
	cppcheck --quiet --enable=all \
                 -ILetheCommon/include \
                 -ILetheThreadComm/include \
                 -ILetheProcessComm/include \
                 -ILetheSocketComm/include \
                 -ILetheThreadUtil/include \
                 -IthirdParty/mct/include \
                 -IthirdParty/catch/include \
                 LetheCommon/test \
                 LetheThreadComm/test \
                 LetheProcessComm/test \
                 LetheSocketComm/test \
                 LetheThreadUtil/test \
          2>&1 | tee checkTest.log

clean:
	cd thirdParty; make clean
	cd LetheCommon; make clean
	cd LetheThreadComm; make clean
	cd LetheProcessComm; make clean
	cd LetheSocketComm; make clean
	cd LetheThreadUtil; make clean
