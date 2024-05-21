CC = gcc

CFLAGS += -Wall
CFLAGS += -g

BINS = a.out b.out a_corrigido.out

all: $(BINS)

a.out: a.o
	$(CC) $(CFLAGS) $^ -o $@ $(LOADLIBES)

a.o: a.c

b.out: b.o
	$(CC) $(CFLAGS) $^ -o $@ $(LOADLIBES)

b.o: b.c

a_corrigido.out: a_corrigido.o
	$(CC) $(CFLAGS) $^ -o $@ $(LOADLIBES)

a_corrigido.o: a_corrigido.c

clean:
	$(RM) $(BINS) *.o *.out