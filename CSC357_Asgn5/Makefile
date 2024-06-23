INCLUDEDIR = ~pn-cs357/Given/Talk/include
LIBDIR = ~pn-cs357/Given/Talk/lib64
CC = gcc
CFLAGS = -Wall -g -ansi -pedantic -std=gnu99
LD = gcc
LDFLAGS = -Wall -g -ansi -pedantic -std=gnu99

all: mytalk

mytalk: mytalk.o $(LIBDIR)/libtalk.a
	$(LD) $(LDFLAGS) -L $(LIBDIR) -o mytalk mytalk.o -ltalk -lncurses

mytalk.o: mytalk.c $(INCLUDEDIR)/talk.h
	$(CC) $(CFLAGS) -I $(INCLUDEDIR) -c -o mytalk.o mytalk.c

clean:
	rm *.o mytalk

