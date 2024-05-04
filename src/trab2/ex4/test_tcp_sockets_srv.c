#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "sockets.h"
#include "errors.h"

#define DEFAUT_SERVER_PORT  5000
#define MAX_BUF               64

void handle_client (int socketfd)
{
    char buf[MAX_BUF];
    int nbytesRD;
    while ((nbytesRD = read(socketfd, buf, sizeof(buf))) > 0 ) {
        handle_error_system(writen(STDOUT_FILENO, buf, nbytesRD), "[srv] Writing to stdout");
    }
    handle_error_system(nbytesRD, "[srv] reading from client");
}

int main (int argc, char * argv[])
{
    int serverPort = DEFAUT_SERVER_PORT;

    if (argc == 2) {
        serverPort = atoi(argv[1]);
    }

    if ( serverPort < 1024 ) {
        printf("Port sould be above 1024\n");
        exit(EXIT_FAILURE);		 
    }
           	
    printf("Server using port: %d\n", serverPort);



    int socketfd = tcp_socket_server_init(serverPort);

    handle_error_system(socketfd, "[srv] Server socket init");

    while ( 1 ) {
        int newsocketfd = tcp_socket_server_accept(socketfd);
        handle_error_system(newsocketfd, "[srv] Accept new connection");

        handle_client(newsocketfd);

        handle_error_system(close(newsocketfd), "[srv] closing socket to client");
    }

    return 0;
}