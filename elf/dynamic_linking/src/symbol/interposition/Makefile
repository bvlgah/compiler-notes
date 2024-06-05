.PHONY: clean

libapi1.so: impl1.o
	$(CC) -shared -fPIC -o $@ $^

libapi2.so: impl2.o
	$(CC) -shared -fPIC -o $@ $^

main: main.o libapi1.so
	$(CC) -L. -Wl,-rpath,\$$ORIGIN -o $@ $< -lapi1

clean:
	rm -rf *.o *.so main
