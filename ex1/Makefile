CC = gcc

CFLAGS += -Wall
CFLAGS += -g
LOADLIBES += -lm

BINS = cpu_stress

all: $(BINS)

cpu_stress: cpu_stress.o
cpu_stress.o: cpu_stress.c

clean:
	$(RM) $(BINS) *.o $(SUBMIT_FILENAME)