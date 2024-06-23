CC = gcc
CFLAGS = -Wall -g
LD = gcc
LDFLAGS = -Wall -g

all: uniq
	
uniq: uniq.o rll.o
	$(LD) $(LDFLAGS) -o uniq uniq.o rll.o

uniq.o: uniq.c
	$(CC) $(CFLAGS) -c uniq.c -o uniq.o

rll.o: rll.c
	$(CC) $(CFLAGS) -c rll.c -o rll.o

test: uniq
	echo Testing uniq
	./uniq < TestInputFile
	echo done.

clean:
	rm *.o uniq
