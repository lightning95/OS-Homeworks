all: forking polling

forking: libhelpers 
	gcc forking.c -g -L../lib -I../lib -Wl,-rpath=../lib -std=gnu99 -lhelpers -lbufio -o forking 

polling: libhelpers 
	gcc polling.c -g -L../lib -I../lib -Wl,-rpath=../lib -std=gnu99 -lhelpers -lbufio -o polling 

libhelpers:
	make -C ../lib

clean:
	rm -f ./forking
	rm -f ./polling
