CFLAGS=-g -Wall 
FLAGSTHREAD=-g -Wall -lpthread

all: huangjen.adventure huangjen.buildrooms

huangjen.adventure: huangjen.adventure.c
	gcc -o huangjen.adventure huangjen.adventure.c ${CFLAGSTHREAD}

huangjen.buildrooms: huangjen.buildrooms.c
	gcc -o huangjen.buildrooms huangjen.buildrooms.c ${CFLAGS}

clean:
	rm -f huangjen.adventure
	rm -f huangjen.buildrooms
