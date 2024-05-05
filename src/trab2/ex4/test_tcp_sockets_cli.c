#include "sockets.h"
#include "errors.h"

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>


#define DEFAULT_SERVER_HOST "localhost"
#define DEFAUT_SERVER_PORT  5000
#define MAX_BUF               32

int main (int argc, char * argv[])
{
    char *serverEndpoint  = DEFAULT_SERVER_HOST;
    int   serverPort      = DEFAUT_SERVER_PORT;

    if (argc == 3) {
        serverEndpoint = argv[1];
        serverPort     = atoi(argv[2]);
    }

    printf("client connecting to: %s:%d\n", serverEndpoint, serverPort);


    int socketfd = tcp_socket_client_init(serverEndpoint, serverPort);
    handle_error_system(socketfd, "[cli] Server socket init");

    char *msg = "Ola boa tarde\n";
    
    handle_error_system(writen(socketfd, msg, strlen(msg)), "Writing to server");

    handle_error_system(close(socketfd), "[cli] closing socket to server");

    return 0;
}