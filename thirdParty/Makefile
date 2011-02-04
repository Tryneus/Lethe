prepare: catch mct

catch: catch.tar.gz catch.exception.patch
	tar -zxf catch.tar.gz
	patch -p0 -i catch.exception.patch
	touch catch

# TODO: Make a patch for mct so it doesn't do so much extra crap, we just want the generated headers
mct: mct.tar.gz
	tar -zxf mct.tar.gz
	touch mct
	cd mct; scons install prefix=./
	patch -p0 -i mct.gcc.patch

clean:
	rm -rf catch mct
