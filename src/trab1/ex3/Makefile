CC = gcc

CFLAGS += -Wall
CFLAGS += -g

BINS = count_words.out

all: $(BINS)

count_words.out: count_words.o
	$(CC) $(CFLAGS) $^ -o $@

count_words.o: count_words.c

clean:
	$(RM) $(BINS) *.o *.out