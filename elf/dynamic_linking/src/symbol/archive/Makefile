.PHONY: clean

libapi.a: greet.o println.o strong.o
	ar rcs $@ $^

main: main.o libapi.a
	$(CC) -o $@ $< -L. -lapi

clean:
	rm -rf *.o *.a main
