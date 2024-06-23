CC = gcc
CFLAGS = -g -Wall -ansi -pedantic -std=gnu99
LD = gcc
LDFLAGS = -g -Wall -ansi -pedantic -std=gnu99

all: timeit

timeit: timeit.o
	$(LD) $(LDFLAGS) -o timeit timeit.o

timeit.o: timeit.c
	$(CC) $(CFLAGS) -c -o timeit.o timeit.c

clean:
	rm *.o timeit
