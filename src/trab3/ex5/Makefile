CC = gcc

CFLAGS += -Wall
CFLAGS += -g

BINS = test_tcp_sockets_srv.out test_tcp_sockets_cli.out

all: $(BINS)

test_tcp_sockets_srv.out: test_tcp_sockets_srv.o sockets.o errors.o vector_utils.o
	$(CC) $(CFLAGS) $^ -o $@ $(LOADLIBES)

test_tcp_sockets_srv.o: test_tcp_sockets_srv.c sockets.h errors.h vector_utils.h

test_tcp_sockets_cli.out: test_tcp_sockets_cli.o sockets.o errors.o vector_utils.o
	$(CC) $(CFLAGS) $^ -o $@ $(LOADLIBES)

test_tcp_sockets_cli.o: test_tcp_sockets_cli.c sockets.h errors.h vector_utils.h

sockets.o: sockets.c sockets.h

errors.o: errors.c errors.h

vector_utils.o: vector_utils.c vector_utils.h

clean:
	$(RM) $(BINS) *.o *.out