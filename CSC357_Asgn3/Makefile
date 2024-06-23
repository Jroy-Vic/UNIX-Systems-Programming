CC = gcc
CFLAGS = -Wall -ansi -pedantic -g
LD = gcc
LDFLAGS = -Wall -ansi -pedantic -g

all: hencode

hencode: hencode.o htable.o tree.o link.o
	$(LD) $(LDFLAGS) -o hencode hencode.o htable.o tree.o link.o

hencode.o: hencode.c htable.h
	$(CC) $(CFLAGS) -c -o hencode.o hencode.c

htable.o: htable.c htable.h tree.h link.h
	$(CC) $(CFLAGS) -c -o htable.o htable.c

tree.o: tree.c tree.h link.h
	$(CC) $(CFLAGS) -c -o tree.o tree.c

link.o: link.c link.h
	$(CC) $(CFLAGS) -c -o link.o link.c

clean:
	rm *.o hencode
