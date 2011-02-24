prepare: catch mct

catch: catch.tar.gz
	tar -zxf catch.tar.gz
	touch catch

# This skips the configure step and just uses a hard-coded configure file
mct: mct.tar.gz
	tar -zxf mct.tar.gz
	mkdir mct/include
	cp -r mct/mct mct/include
	cp mct.linux.config.hpp mct/include/mct/config.hpp
	patch -p0 -i mct.gcc.patch

clean:
	rm -rf catch mct
