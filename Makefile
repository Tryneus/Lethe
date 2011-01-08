all:
	cd libCommon; make all
	cd libThreadComm; make all
	cd ThreadCommTest; make all

clean:
	cd libCommon; make clean
	cd libThreadComm; make clean
	cd ThreadCommTest; make clean
