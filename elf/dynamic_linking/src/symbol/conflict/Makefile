.PHONY: clean

main: main.o api.o

libmain.so: main.o api.o
	$(CC) -shared -fPIC -o $@ $^

libapi.so: api.o
	$(CC) -shared -fPIC -o $@ $^

greet: main.o libapi.so
	$(CC) -o $@ -L. -lapi $<

clean:
	rm -rf main *.o *.so greet
