run: lib
	gcc -Wall -std=c99 -shared helpers.o -o libhelpers.so

lib: clean
	gcc -Wall -std=c99 -c -fPIC helpers.c -o helpers.o

clean:
	rm -f libhelpers.so
	rm -f helpers.o
