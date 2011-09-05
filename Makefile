
all: prepare
	cd LetheCommon; make all
	cd LetheThreadComm; make all
	cd LetheProcessComm; make all
	cd LetheSocketComm; make all
	cd LetheThreadUtil; make all

prepare:
	cd thirdParty; make prepare

# install is used to install the kernel module needed for mutex/semaphore/event auto-reset on linux
install: prepare
	cd LetheCommon; make install

check: prepare
	cd LetheCommon; make check
	cd LetheThreadComm; make check
	cd LetheProcessComm; make check
	cd LetheSocketComm; make check
	cd LetheThreadUtil; make check

valCheck: prepare
	cd LetheCommon; make valCheck
	cd LetheThreadComm; make valCheck
	cd LetheProcessComm; make valCheck
	cd LetheSocketComm; make valCheck
	cd LetheThreadUtil; make valCheck

cppcheck: prepare
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

cppcheckTest: prepare
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
