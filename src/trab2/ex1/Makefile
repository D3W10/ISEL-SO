CC = gcc

CFLAGS += -Wall
CFLAGS += -g

BINS = pc_info.out

all: $(BINS)

pc_info.out: pc_info.o
	$(CC) $(CFLAGS) $^ -o $@

pc_info.o: pc_info.c

clean:
	$(RM) $(BINS) *.o *.out