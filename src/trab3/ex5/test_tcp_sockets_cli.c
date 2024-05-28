#include "sockets.h"
#include "errors.h"
#include "vector_utils.h"

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#define DEFAULT_SERVER_HOST "localhost"
#define DEFAUT_SERVER_PORT         5000
#define MAX_BUF                      32
#define LOWER_LIMIT                   0
#define UPPER_LIMIT                 100
#define MIN                          50
#define MAX                         100

int main(int argc, char * argv[])
{
    char *serverEndpoint  = DEFAULT_SERVER_HOST;
    int   serverPort      = DEFAUT_SERVER_PORT;

    if (argc < 2) {
        printf("Usage: %s <vector_size> [<server_endpoint> <server_port>]\n", argv[0]);
        return -1;
    }

    int values_sz = atoi(argv[1]);

    if (argc == 4) {
        serverEndpoint = argv[2];
        serverPort     = atoi(argv[3]);
    }

    printf("client connecting to: %s:%d\n", serverEndpoint, serverPort);

    int socketfd = tcp_socket_client_init(serverEndpoint, serverPort);
    handle_error_system(socketfd, "[cli] Server socket init");

    printf("Initializing a vector of %d values\n", values_sz);

    // allocate a vector of initial values
    int *values = malloc(sizeof(int) * values_sz);
    if (values == NULL) {
        fprintf(stderr, "Erro malloc\n");
        return -1;
    }

    // allocate a subvector where will be store values in a interval of values
    int *subvalues = malloc(sizeof(int) * values_sz);
    if (subvalues == NULL) {
        fprintf(stderr, "Erro malloc\n");
        return -1;
    }

    int info[] = { values_sz, MIN, MAX };
    int subvalues_size = 0;

    // initiate initial array of values 
    vector_init_rand(values, values_sz, LOWER_LIMIT, UPPER_LIMIT);

    handle_error_system(writen(socketfd, info, sizeof(info)), "Writing to server");
    handle_error_system(writen(socketfd, values, values_sz * sizeof(int)), "Writing to server");

    int buf[MAX_BUF], finish = 0;

    while (read(socketfd, buf, sizeof(buf)) <= 0);
    cpy_buffer(subvalues, buf, sizeof(buf) / sizeof(int), &subvalues_size);

    while (!finish && read(socketfd, buf, sizeof(buf)) > 0)
        finish = cpy_buffer(subvalues, buf, sizeof(buf) / sizeof(int), &subvalues_size);

    printf("Values between [%d..%d]: %d\n", info[0], info[1], subvalues_size);

    handle_error_system(close(socketfd), "[cli] closing socket to server");

    free(values);
    free(subvalues);

    return 0;
}