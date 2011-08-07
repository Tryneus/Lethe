prepare: catch mct

catch: catch.tar.gz
	tar -zxf catch.tar.gz
	touch catch

# This skips the configure step and just uses a hard-coded configure file
# Only extract the header files
mct: mct.tar.gz
	tar --strip-components=1 -xvf mct.tar.gz mct/mct
	cp mct.linux.config.hpp mct/config.hpp
	patch -p0 -i mct.gcc.patch

clean:
	rm -rf catch mct
