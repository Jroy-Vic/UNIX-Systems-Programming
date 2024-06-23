CC = gcc
CFLAGS = -Wall -ansi -pedantic -g
LD = gcc
LDFLAGS = -Wall -ansi -pedantic -g

all: htable

htable: htable.o tree.o link.o
	$(LD) $(LDFLAGS) -o htable htable.o tree.o link.o

htable.o: htable.c tree.h link.h
	$(CC) $(CFLAGS) -c -o htable.o htable.c

tree.o: tree.c tree.h link.h
	$(CC) $(CFLAGS) -c -o tree.o tree.c

link.o: link.c link.h
	$(CC) $(CFLAGS) -c -o link.o link.c

clean:
	rm *.o htable
