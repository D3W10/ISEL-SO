#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include "sockets.h"
#include "errors.h"
#include "thread_pool.h"
#include "countdown.h"

#define DEFAUT_SERVER_PORT             5000
#define MAX_BUF                          64
#define MAX_CLIENTS                      10
#define MAX_THREADS                       5
#define UNIX_SOCKET_PATH "/tmp/unix_socket"

typedef struct {
    int socketfd;
    threadpool_t *pool;
} client_args;

typedef struct {
    int* v;
    int size;
    int index;
    int min;
    int max;
    int* res;
    int res_count;
    countdown_t *cd;
} thread_args;

void process_client(int socketfd, threadpool_t *pool);
int vector_get_in_range_with_thread_pool (int v[], int v_sz, int sv[], int min, int max, threadpool_t *tp);
void tcp_accept(int tcpSocketfd, threadpool_t *pool);
void unix_accept(int unixSocketfd, threadpool_t *pool);

void range_child(thread_args* args) {
    for (int j = args->index * args->size; j < (args->index + 1) * args->size; j++) {
        if (args->v[j] >= args->min && args->v[j] <= args->max)
            args->res[args->res_count++] = args->v[j];
    }

    countdown_down(args->cd);
}

void* handle_client(void* args) {
    client_args *p_args = (client_args *)args;

    process_client(p_args->socketfd, p_args->pool);

    handle_error_system(close(p_args->socketfd), "[srv] closing socket to client");
    free(p_args);

    pthread_exit(NULL);
}

void process_client(int socketfd, threadpool_t *pool) {
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
        threadpool_destroy(pool);
        exit(EXIT_FAILURE);
    }

    int *allVals = malloc(sizeof(int) * cnt), allValsSize = 0;
    if (allVals == NULL) {
        handle_error_system(close(socketfd), "[srv] closing socket to client");
        fprintf(stderr, "Erro malloc\n");
        threadpool_destroy(pool);
        exit(EXIT_FAILURE);
    }

    while (cnt != allValsSize && (readBytes = readn(socketfd, buf, (batchCount < nFullBatch ? MAX_BUF : cnt % MAX_BUF) * sizeof(int))) > 0) {
        for (int i = 0; i < readBytes / sizeof(int); i++)
            allVals[allValsSize++] = buf[i];

        batchCount++;
    }

    vecSize = vector_get_in_range_with_thread_pool(allVals, cnt, vec, min, max, pool);

    handle_error_system(writen(socketfd, &vecSize, sizeof(int)), "Writing to client");
    handle_error_system(writen(socketfd, vec, sizeof(int) * vecSize), "Writing to client");

    free(allVals);
    free(vec);
}

int vector_get_in_range_with_thread_pool (int v[], int v_sz, int sv[], int min, int max, threadpool_t *tp) {
    thread_args targs[MAX_THREADS];
    countdown_t cd;
    int num_elements = 0;
    int subarray_size = v_sz / MAX_THREADS;

    countdown_init(&cd, MAX_THREADS);

    for (int i = 0; i < MAX_THREADS; i++) {
        targs[i].v = v;
        targs[i].size = subarray_size;
        targs[i].min = min;
        targs[i].max = max;
        targs[i].index = i;
        targs[i].res = malloc(sizeof(int) * subarray_size);
        targs[i].res_count = 0;
        targs[i].cd = &cd;

        if (targs[i].res == NULL) {
            threadpool_destroy(tp);
            fprintf(stderr, "Erro malloc\n");
            return -1;
        }

        if (threadpool_submit(tp, (void *(*)(void *))range_child, &targs[i]) != 0) {
            threadpool_destroy(tp);
            fprintf(stderr, "Erro threadpool_submit\n");
            return -1;
        }
    }

    countdown_wait(&cd);
    countdown_destroy(&cd);

    for (int i = 0; i < MAX_THREADS; i++) {
        for (int j = 0; j < targs[i].res_count; j++)
            sv[num_elements++] = targs[i].res[j];

        free(targs[i].res);
    }

    return num_elements;
}

int main(int argc, char *argv[]) {
    int serverPort = DEFAUT_SERVER_PORT;

    if (argc == 2)
        serverPort = atoi(argv[1]);

    if (serverPort < 1024) {
        fprintf(stderr, "Port should be above 1024\n");
        exit(EXIT_FAILURE);
    }

    threadpool_t pool;
    threadpool_init(&pool, MAX_CLIENTS, MAX_THREADS);

    printf("Server running on port %d\n", serverPort);

    int tcpSocketfd = tcp_socket_server_init(serverPort);
    handle_error_system(tcpSocketfd, "[srv] TCP server socket init");

    int unixSocketfd = un_socket_server_init(UNIX_SOCKET_PATH);
    handle_error_system(unixSocketfd, "[srv] UNIX server socket init");

    pid_t child = fork();
    if (child < 0) {
        fprintf(stderr, "Error while forking");
        exit(EXIT_FAILURE);
    }
    else if (child == 0) {
        unix_accept(unixSocketfd, &pool);
        exit(EXIT_SUCCESS);
    }

    tcp_accept(tcpSocketfd, &pool);

    return 0;
}

void tcp_accept(int tcpSocketfd, threadpool_t *pool) {
    while (1) {
        int newsocketfd = tcp_socket_server_accept(tcpSocketfd);

        handle_error_system(newsocketfd, "[srv] Accept new connection");

        client_args *p_args = malloc(sizeof(client_args));
        p_args->socketfd = newsocketfd;
        p_args->pool = pool;

        pthread_t pth;
        pthread_attr_t attr;

        pthread_attr_init(&attr);
        pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);

        if (pthread_create(&pth, &attr, handle_client, p_args) != 0) {
            handle_error_system(close(newsocketfd), "[srv] closing socket to client");
            fprintf(stderr, "Error creating thread\n");
            threadpool_destroy(pool);
            exit(EXIT_FAILURE);
        }
    }
}

void unix_accept(int unixSocketfd, threadpool_t *pool) {
    while (1) {
        int newsocketfd = un_socket_server_accept(unixSocketfd);

        handle_error_system(newsocketfd, "[srv] Accept new connection");

        client_args *p_args = malloc(sizeof(client_args));
        p_args->socketfd = newsocketfd;
        p_args->pool = pool;

        pthread_t pth;
        pthread_attr_t attr;

        pthread_attr_init(&attr);
        pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);

        if (pthread_create(&pth, &attr, handle_client, p_args) != 0) {
            handle_error_system(close(newsocketfd), "[srv] closing socket to client");
            fprintf(stderr, "Error creating thread\n");
            threadpool_destroy(pool);
            exit(EXIT_FAILURE);
        }
    }
}