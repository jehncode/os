CFLAGS=-Wall -std=c99
CFLAGSTHREAD= -lpthread

all: smallsh

smallsh: smallsh.c
	gcc -o smallsh smallsh.c ${CFLAGS}

clean:
	rm -f smallsh 
