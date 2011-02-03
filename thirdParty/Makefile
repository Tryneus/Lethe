prepare: catch

catch: catch.tar.gz catch.exception.patch
	tar -zxvf catch.tar.gz
	patch -p0 -i catch.exception.patch
	touch catch

clean:
	rm -rf catch
