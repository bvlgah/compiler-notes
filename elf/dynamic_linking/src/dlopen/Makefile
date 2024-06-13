.PHONY: clean

libapi.so: api.c
	$(CC) $(CFLAGS) -shared -fPIC -o $@ $^ -ldl

main: main.o libapi.so
	$(CC) $(CFLAGS) -fPIE -o main main.o -ldl

clean:
	rm -rf *.o *.so main
