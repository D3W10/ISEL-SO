CC = gcc

CFLAGS += -Wall
CFLAGS += -g

BINS = prog.out

all: $(BINS)

prog.out: prog.o
	$(CC) $(CFLAGS) $^ -o $@ $(LOADLIBES)

prog.o: prog.c

clean:
	$(RM) $(BINS) *.o *.out