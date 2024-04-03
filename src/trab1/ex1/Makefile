CC = gcc

CFLAGS += -Wall
CFLAGS += -g
LOADLIBES += -lm

BINS = cpu_stress.out

all: $(BINS)

cpu_stress.out: cpu_stress.o
	$(CC) $(CFLAGS) $^ -o $@ $(LOADLIBES)

cpu_stress.o: cpu_stress.c

clean:
	$(RM) $(BINS) *.o *.out