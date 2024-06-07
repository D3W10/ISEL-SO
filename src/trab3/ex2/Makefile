CC = gcc

CFLAGS += -Wall
CFLAGS += -g

BINS = ex2.out

all: $(BINS)

ex2.out: ex2.o
	$(CC) $(CFLAGS) -pthread $^ -o $@ $(LOADLIBES)

ex2.o: ex2.c

clean:
	$(RM) $(BINS) *.o *.out