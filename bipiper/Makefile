all: forking

forking: libhelpers 
	gcc forking.c -g -L../lib -I../lib -Wl,-rpath=../lib -std=gnu99 -lhelpers -lbufio -o forking 

libhelpers:
	make -C ../lib

clean:
	rm -f ./forking
