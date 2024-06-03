CC = gcc

CFLAGS += -Wall
CFLAGS += -g
LOADLIBES += -lm

BINS = ex4.out

all: $(BINS)

ex4.out: ex4.o
	$(CC) $(CFLAGS) $^ -o $@ $(LOADLIBES)

ex4.o: ex4.c

clean:
	$(RM) $(BINS) *.o *.out