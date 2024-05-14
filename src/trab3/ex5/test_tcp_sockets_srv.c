#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include "sockets.h"
#include "errors.h"

#define DEFAUT_SERVER_PORT    5000
#define MAX_BUF                 64

void process_client(int* socketfd);

void handle_client(int* socketfd) {
    process_client(socketfd);

    handle_error_system(close(*socketfd), "[srv] closing socket to client");
    free(socketfd);
}

void process_client(int* socketfd) {
    int buf[MAX_BUF];
    int nbytesRD;
    int vecCapacity = MAX_BUF * 2;

    int* vec = malloc(sizeof(int) * vecCapacity), vecSize = 0;
    if (vec == NULL) {
        handle_error_system(close(*socketfd), "[srv] closing socket to client");
        fprintf(stderr, "Erro malloc\n");
        exit(EXIT_FAILURE);
    }

    read(*socketfd, buf, sizeof(buf));
    int cnt = buf[0], min = buf[1], max = buf[2], globalCount = 0, stopLoop = 0;

    while ((nbytesRD = read(*socketfd, buf, sizeof(buf))) > 0) {
        for (int i = 0; i < sizeof(buf) / sizeof(int) && !stopLoop; i++) {
            if (buf[i] >= min && buf[i] <= max) {
                if (vecSize + 1 > vecCapacity) {
                    vecCapacity += MAX_BUF * 2;

                    if ((vec = realloc(vec, sizeof(int) * vecCapacity)) == NULL) {
                        handle_error_system(close(*socketfd), "[srv] closing socket to client");
                        fprintf(stderr, "Erro malloc\n");
                        exit(EXIT_FAILURE);
                    }
                }

                vec[vecSize++] = buf[i];
            }

            if (++globalCount >= cnt)
                stopLoop = 1;
        }
        if (stopLoop)
            break;
    }

    handle_error_system(writen(*socketfd, vec, sizeof(int) * vecCapacity), "Writing to client");
    free(vec);

    handle_error_system(nbytesRD, "[srv] reading from client");
}

int main(int argc, char* argv[]) {
    int serverPort = DEFAUT_SERVER_PORT;

    if (argc < 2) {
        printf("Usage: %s <process_switch> [<server_port>]\n\t<process_switch>\tEither -p or -t (processes or threads)\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    int useThreads = !strcmp(argv[1], "-t");

    if (argc == 3)
        serverPort = atoi(argv[2]);

    if (serverPort < 1024) {
        printf("Port should be above 1024\n");
        exit(EXIT_FAILURE);
    }
           	
    printf("Server running on port %d\n", serverPort);

    int socketfd = tcp_socket_server_init(serverPort);

    handle_error_system(socketfd, "[srv] Server socket init");

    while (1) {
        int newsocketfd = tcp_socket_server_accept(socketfd);

        handle_error_system(newsocketfd, "[srv] Accept new connection");

        int* socket_fd_cpy = malloc(sizeof(int));
        *socket_fd_cpy = newsocketfd;

        if (!useThreads) {
            pid_t pid = fork();

            if (pid < 0) {
                handle_error_system(close(newsocketfd), "[srv] closing socket to client");
                fprintf(stderr, "Error creating process\n");
                exit(EXIT_FAILURE);
            }
            else if (pid == 0) {
                handle_client(socket_fd_cpy);
                exit(EXIT_SUCCESS);
            }
        }
        else {
            pthread_t pth;

            if (pthread_create(&pth, NULL, (void *(*)(void *))handle_client, (void *)socket_fd_cpy) != 0) {
                handle_error_system(close(newsocketfd), "[srv] closing socket to client");
                fprintf(stderr, "Error creating thread\n");
                exit(EXIT_FAILURE);
            }
        }
    }

    return 0;
}