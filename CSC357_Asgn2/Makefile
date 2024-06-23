CC = gcc
CFLAGS = -Wall -pedantic -ansi -g
LD = gcc
LDFLAGS = -Wall -pedantic -ansi -g

all: fw

fw: fw.o hash.o rll.o
	$(LD) $(LDFLAGS) -o fw fw.o hash.o rll.o 

fw.o: fw.c hash.h rll.h
	$(CC) $(CFLAGS) -o fw.o -c fw.c

hash.o: hash.c hash.h
	$(CC) $(CFLAGS) -o hash.o -c hash.c

rll.o: rll.c rll.h
	$(CC) $(CFLAGS) -o rll.o -c rll.c

clean:
	rm *.o fw
