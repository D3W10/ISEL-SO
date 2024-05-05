CC = gcc
CFLAGS = -g -Wall

BINS = test_tcp_sockets_srv.out test_tcp_sockets_cli.out

all: $(BINS)

test_tcp_sockets_srv.out: test_tcp_sockets_srv.o sockets.o errors.o

test_tcp_sockets_srv.o: test_tcp_sockets_srv.c sockets.h errors.h


test_tcp_sockets_cli.out: test_tcp_sockets_cli.o sockets.o errors.o

test_tcp_sockets_cli.o: test_tcp_sockets_cli.c sockets.h errors.h



sockets.o: sockets.c sockets.h

errors.o: errors.c errors.h

clean:
	$(RM) *.o $(BINS)