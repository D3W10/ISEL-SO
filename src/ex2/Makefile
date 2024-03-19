CC = gcc

CFLAGS += -Wall
CFLAGS += -g

BINS = prog

all: $(BINS)

prog: prog.out
	./prog.out

prog.out: prog.o
	$(CC) $(CFLAGS) $^ -o $@

prog.o: prog.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	$(RM) $(BINS) *.o *.out