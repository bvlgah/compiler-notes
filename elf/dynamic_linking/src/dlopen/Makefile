.PHONY: clean

CFLAGS += -Wl,-z,defs -fPIC

libbar.so: bar.o
	$(CC) $(CFLAGS) -shared -o $@ $<

libfoo.so: foo.o
	$(CC) $(CFLAGS) -shared -L. -o $@ $< -lutil

libutil.so: util.o
	$(CC) $(CFLAGS) -shared -o $@ $< -ldl

libdltest.so: test.o libutil.so
	$(CC) $(CFLAGS) -Wl,-rpath,\$$ORIGIN -L. -shared -o $@ $< -ldl -lutil

main: main.o libutil.so libdltest.so libfoo.so libbar.so
	$(CC) $(CFLAGS) -Wl,-rpath,\$$ORIGIN -fPIE -L. -o $@ $< -lutil

clean:
	rm -rf *.o *.so main
