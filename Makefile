CC = gcc

CFLAGS += -Wall 
CFLAGS += -g

SUBMIT_FILENAME = Apoio_T1.tar.gz

BINS = vector_stat_seq

all: $(BINS)

vector_stat_seq: vector_stat_seq.o
vector_stat_seq.o: vector_stat_seq.c


clean:
	$(RM) $(BINS) *.o $(SUBMIT_FILENAME)

submit: clean
	tar -czvf $(SUBMIT_FILENAME) *