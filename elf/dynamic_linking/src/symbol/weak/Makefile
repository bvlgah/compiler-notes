.PHONY: clean

main_weak1: main.o impl_weak1.o impl_weak2.o
	$(CC) -o $@ $^

main_weak2: main.o impl_weak2.o impl_weak1.o
	$(CC) -o $@ $^

main_strong: main.o impl_strong.o impl_weak1.o impl_weak2.o
	$(CC) -o $@ $^

clean:
	rm -rf *.o main_weak1 main_weak2 main_strong
