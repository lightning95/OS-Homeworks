all:
	LD_LIBRARY_PATH="../lib"
	gcc bufcat.c -L../lib/ -Wl,-rpath=../lib -lbufio -o bufcat

clean:
	rm bufcat
	