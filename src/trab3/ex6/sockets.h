#ifndef SOCKETS_H_
#define SOCKETS_H_

#include <stddef.h>

int tcp_socket_server_init(int serverPort);
int tcp_socket_server_accept(int serverSocket);
int tcp_socket_client_init(const char *host, int port);

int un_socket_server_init(const char *serverEndPoint);
int un_socket_server_accept(int serverSocket);
int un_socket_client_init(const char *serverEndPoint);

int readn(int fd, void *buf, size_t count);
int writen(int fd, const void *buf, size_t count);

#endif