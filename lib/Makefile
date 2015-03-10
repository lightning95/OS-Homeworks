run: lib
	gcc -shared helpers.o -o libhelpers.so

lib: clean
	gcc -c -fPIC -std=c99 helpers.c -o helpers.o

clean:
	rm -f libhelpers.so
	rm -f helpers.o