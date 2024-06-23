CC = gcc
CFLAGS = -Wall -g -ansi -pedantic
LD = gcc
LDFALGS = -Wall -g -ansi -pedantic

all: forkit tryit

forkit: forkit.o
	$(LD) $(LDFLAGS) forkit.o -o forkit

tryit: tryit.o
	$(LD) $(LDFLAGS) tryit.o -o tryit

forkit.o: forkit.c
	$(CC) $(CFLAGS) -c forkit.c -o forkit.o

tryit.o: tryit.c
	$(CC) $(CFLAGS) -c tryit.c -o tryit.o

clean:
	rm *.o forkit tryit
