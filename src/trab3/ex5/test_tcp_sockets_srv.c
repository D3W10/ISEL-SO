#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include "sockets.h"
#include "errors.h"

#define DEFAUT_SERVER_PORT             5000
#define MAX_BUF                          64
#define UNIX_SOCKET_PATH "/tmp/unix_socket"

typedef struct {
    int socketfd;
    int useThreads;
} client_args;

typedef struct {
    int* v;
    int size;
    int index;
    int min;
    int max;
    int* res;
    int res_count;
} thread_args;

void process_client(int socketfd, int useThreads);
int vector_get_in_range_with_processes(int v[], int v_sz, int sv[], int min, int max, int n_processes);
int vector_get_in_range_with_threads(int v[], int v_sz, int sv[], int min, int max, int n_threads);
void tcp_accept(int tcpSocketfd, int useThreads);
void unix_accept(int unixSocketfd, int useThreads);

void range_child(thread_args* args) {
    for (int j = args->index * args->size; j < (args->index + 1) * args->size; j++) {
        if (args->v[j] >= args->min && args->v[j] <= args->max)
            args->res[args->res_count++] = args->v[j];
    }

    pthread_exit(NULL);
}

void* handle_client(void* args) {
    client_args *p_args = (client_args *)args;

    process_client(p_args->socketfd, p_args->useThreads);

    handle_error_system(close(p_args->socketfd), "[srv] closing socket to client");
    free(p_args);

    pthread_exit(NULL);
}

void process_client(int socketfd, int useThreads) {
    int buf[MAX_BUF];
    int cnt, min, max, nFullBatch, batchCount = 0, readBytes;

    readn(socketfd, buf, 3 * sizeof(int));
    cnt = buf[0];
    min = buf[1];
    max = buf[2];
    nFullBatch = cnt / MAX_BUF;

    int *vec = malloc(sizeof(int) * cnt), vecSize;
    if (vec == NULL) {
        handle_error_system(close(socketfd), "[srv] closing socket to client");
        fprintf(stderr, "Erro malloc\n");
        exit(EXIT_FAILURE);
    }

    int *allVals = malloc(sizeof(int) * cnt), allValsSize = 0;
    if (allVals == NULL) {
        handle_error_system(close(socketfd), "[srv] closing socket to client");
        fprintf(stderr, "Erro malloc\n");
        exit(EXIT_FAILURE);
    }

    while (cnt != allValsSize && (readBytes = readn(socketfd, buf, (batchCount < nFullBatch ? MAX_BUF : cnt % MAX_BUF) * sizeof(int))) > 0) {
        for (int i = 0; i < readBytes / sizeof(int); i++)
            allVals[allValsSize++] = buf[i];

        batchCount++;
    }

    if (!useThreads)
        vecSize = vector_get_in_range_with_processes(allVals, cnt, vec, min, max, 1);
    else
        vecSize = vector_get_in_range_with_threads(allVals, cnt, vec, min, max, 1);

    handle_error_system(writen(socketfd, &vecSize, sizeof(int)), "Writing to client");
    handle_error_system(writen(socketfd, vec, sizeof(int) * vecSize), "Writing to client");

    free(allVals);
    free(vec);
}

// Função do Trabalho 1 - Exercício 5
int vector_get_in_range_with_processes(int v[], int v_sz, int sv[], int min, int max, int n_processes) {
    int pipefd[n_processes][2];
    pid_t retfork[n_processes];
    int subarray_size = v_sz / n_processes;

    for (int i = 0; i < n_processes; i++){
        if (pipe(pipefd[i]) == -1) {
            perror("pipe");
            exit(1);
        }

        retfork[i] = fork(); // Create child process

        if (retfork[i] < 0) { // Error creating child process
            perror("fork");
            exit(1);
        }
        else if (retfork[i] == 0) { // Code to be executed by the child process
            close(pipefd[i][0]);

            for (int j = i * subarray_size; j < (i + 1) * subarray_size; j++) {
                if (v[j] >= min && v[j] <= max)
                    write(pipefd[i][1], &v[j], sizeof(int));  // Send element to parent process
            }

            close(pipefd[i][1]);
            exit(0);
        }

        close(pipefd[i][1]);
    }

    int num_elements = 0;
    for (int i = 0; i < n_processes; i++) { // Read values from child processes
        int element;

        while (read(pipefd[i][0], &element, sizeof(int)) > 0) {
            sv[num_elements] = element;
            num_elements++;
        }

        close(pipefd[i][0]);
    }

    return num_elements;
}

// Função do Trabalho 2 - Exercício 3
int vector_get_in_range_with_threads(int v[], int v_sz, int sv[], int min, int max, int n_threads) {
    pthread_t th[n_threads];
    thread_args targs[n_threads];
    int num_elements = 0;
    int subarray_size = v_sz / n_threads;

    for (int i = 0; i < n_threads; i++) {
        targs[i].v = v;
        targs[i].size = subarray_size;
        targs[i].min = min;
        targs[i].max = max;
        targs[i].index = i;
        targs[i].res = malloc(sizeof(int) * subarray_size);
        targs[i].res_count = 0;

        if (targs[i].res == NULL)
            fprintf(stderr, "Erro malloc\n");
        else if (pthread_create(&th[i], NULL, (void *(*)(void *))range_child, (void *)&targs[i]) != 0) {
            fprintf(stderr, "Error creating thread\n");
            return 1;
        }
    }

    for (int i = 0; i < n_threads; i++) {
        if (pthread_join(th[i], NULL) != 0) {
            fprintf(stderr, "Error joining thread\n");
            return 1;
        }

        for (int j = 0; j < targs[i].res_count; j++)
            sv[num_elements++] = targs[i].res[j];

        free(targs[i].res);
    }

    return num_elements;
}

int main(int argc, char *argv[]) {
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

    int tcpSocketfd = tcp_socket_server_init(serverPort);
    handle_error_system(tcpSocketfd, "[srv] TCP server socket init");

    int unixSocketfd = un_socket_server_init(UNIX_SOCKET_PATH);
    handle_error_system(unixSocketfd, "[srv] UNIX server socket init");

    pid_t child = fork();
    if (child < 0) {
        perror("fork");
        exit(1);
    }
    else if (child == 0) {
        unix_accept(unixSocketfd, useThreads);
        exit(0);
    }

    tcp_accept(tcpSocketfd, useThreads);

    return 0;
}

void tcp_accept(int tcpSocketfd, int useThreads) {
    while (1) {
        int newsocketfd = tcp_socket_server_accept(tcpSocketfd);

        handle_error_system(newsocketfd, "[srv] Accept new connection");

        client_args *p_args = malloc(sizeof(client_args));
        p_args->socketfd = newsocketfd;
        p_args->useThreads = useThreads;

        pthread_t pth;
        pthread_attr_t attr;

        pthread_attr_init(&attr);
        pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);

        if (pthread_create(&pth, &attr, handle_client, p_args) != 0) {
            handle_error_system(close(newsocketfd), "[srv] closing socket to client");
            fprintf(stderr, "Error creating thread\n");
            exit(EXIT_FAILURE);
        }
    }
}

void unix_accept(int unixSocketfd, int useThreads) {
    while (1) {
        int newsocketfd = un_socket_server_accept(unixSocketfd);

        handle_error_system(newsocketfd, "[srv] Accept new connection");

        client_args *p_args = malloc(sizeof(client_args));
        p_args->socketfd = newsocketfd;
        p_args->useThreads = useThreads;

        pthread_t pth;
        pthread_attr_t attr;

        pthread_attr_init(&attr);
        pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);

        if (pthread_create(&pth, &attr, handle_client, p_args) != 0) {
            handle_error_system(close(newsocketfd), "[srv] closing socket to client");
            fprintf(stderr, "Error creating thread\n");
            exit(EXIT_FAILURE);
        }
    }
}