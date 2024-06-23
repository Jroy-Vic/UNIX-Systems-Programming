CC = gcc
CFLAGS = -Wall -g -ansi -pedantic -std=gnu99
LD = gcc
LDFLAGS = -Wall -g -ansi -pedantic -std=gnu99

all: mypwd

mypwd: mypwd.o dir.o
	$(LD) $(LDFLAGS) -o mypwd mypwd.o dir.o

mypwd.o: mypwd.c dir.h
	$(CC) $(CFLAGS) -c -o mypwd.o mypwd.c

dir.o: dir.c dir.h
	$(CC) $(CFLAGS) -c -o dir.o dir.c

clean:
	rm *.o mypwd
