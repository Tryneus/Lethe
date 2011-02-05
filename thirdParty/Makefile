prepare: catch mct

catch: catch.tar.gz catch.exception.patch
	tar -zxf catch.tar.gz
	patch -p0 -i catch.exception.patch
	touch catch

# TODO: This skips the configure step and just uses a hard-coded configure file
mct: mct.tar.gz
	tar -zxf mct.tar.gz
	mkdir mct/include
	cp -r mct/mct mct/include
	cp mct.linux.config.hpp mct/include/mct/config.hpp
#	touch mct
#	cd mct; scons install prefix=./
	patch -p0 -i mct.gcc.patch

clean:
	rm -rf catch mct
