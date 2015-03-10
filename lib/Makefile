run: lib
	gcc -Wall -shared helpers.o -o libhelpers.so

lib: clean
	gcc -Wall -c -fPIC helpers.c -o helpers.o

clean:
	rm -f libhelpers.so
	rm -f helpers.o