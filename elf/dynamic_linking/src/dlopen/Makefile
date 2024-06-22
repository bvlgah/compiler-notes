.PHONY: clean

.PHONY: all

CFLAGS += -Wl,-z,defs -fPIC

EXE := main switch_main_module dlopen_promotion dlclose

all: $(EXE)

libbar.so: bar.o
	$(CC) $(CFLAGS) -shared -o $@ $<

libfoo.so: foo.o libutil.so
	$(CC) $(CFLAGS) -Wl,-rpath,\$$ORIGIN -shared -L. -o $@ $< -lutil

libutil.so: util.o
	$(CC) $(CFLAGS) -shared -o $@ $< -ldl

libdltest.so: test.o libutil.so
	$(CC) $(CFLAGS) -Wl,-rpath,\$$ORIGIN -L. -shared -o $@ $< -ldl -lutil

main: main.o libutil.so libdltest.so libfoo.so libbar.so
	$(CC) $(CFLAGS) -Wl,-rpath,\$$ORIGIN -fPIE -L. -o $@ $< -lutil

switch_main_module: switch_main_module.o libutil.so libfoo.so libbar.so
	$(CC) $(CFLAGS) -Wl,-rpath,\$$ORIGIN -fPIE -L. -o $@ $< -lutil

dlopen_promotion: dlopen_promotion.o libfoo.so
	$(CC) $(CFLAGS) -Wl,-rpath,\$$ORIGIN -fPIE -o $@ $<

dlclose: dlclose.o libbar.so
	$(CC) $(CFLAGS) -Wl,-rpath,\$$ORIGIN -fPIE -o $@ $<

clean:
	rm -rf *.o *.so $(EXE)
