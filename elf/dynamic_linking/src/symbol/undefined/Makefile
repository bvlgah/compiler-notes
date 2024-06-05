.PHONY: clean

weak: weak.o

weak_static: weak.o
	$(CC) -static -o $@ $^

strong_error: strong.o

strong: strong.o
	$(CC) -Wl,-z,undefs -o $@ $^

strong_lld: strong.o
	$(CC) -Wl,-use-ld=lld -Wl,-z,undefs -o $@ $^

libstrong.so: strong.o
	$(CC) -shared -fPIC -o $@ $^

libstrong_error.so: strong.o
	$(CC) -Wl,-z,defs -shared -fPIC -o $@ $^

main: main.o libstrong.so
	$(CC) -Wl,-fuse-ld=lld -Wl,-z,undefs -Wl,--allow-shlib-undefined -L. \
		-Wl,--rpath,\$$ORIGIN -lstrong -o $@ $<

clean:
	rm -rf *.o *.so weak weak_static strong strong_lld main
