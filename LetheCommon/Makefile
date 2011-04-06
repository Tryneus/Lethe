all:
	cd src; make all
	cd module; make all

# Leaving this commented out right now since I don't trust it enough for people to install
install:
#	cd module; make install

runTest: all
	cd test; make runTest

valTest: all
	cd test; make valTest

clean:
	cd src; make clean
	cd test; make clean
	cd module; make clean
