run: lib
	gcc -Wall -std=c99 -shared helpers.o -o libhelpers.so
	gcc -Wall -std=c99 -shared bufio.o -o libbufio.so
lib: clean
	gcc -Wall -std=c99 -c -fPIC helpers.c -o helpers.o
	gcc -Wall -std=c99 -c -fPIC bufio.c -o bufio.o

clean:
	rm -f libhelpers.so
	rm -f libbufio.so
	rm -f helpers.o
	rm -f bufio.o
	