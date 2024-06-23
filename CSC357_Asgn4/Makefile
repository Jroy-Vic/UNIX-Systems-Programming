CC = gcc
CFLAGS = -Wall -g -ansi -pedantic -std=gnu99
LD = gcc
LDFLAGS = -Wall -g -ansi -pedantic -std=gnu99

all: mytar

mytar: mytar.o tar.o
	$(LD) $(LDFLAGS) -o mytar mytar.o tar.o

mytar.o: mytar.c tar.h
	$(CC) $(CFLAGS) -c -o mytar.o mytar.c

tar.o: tar.c tar.h
	$(CC) $(CFLAGS) -c -o tar.o tar.c

clean:
	rm *.o mytar

