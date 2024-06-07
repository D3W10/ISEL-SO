#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include "sockets.h"
#include "errors.h"
#include "thread_pool.h"
#include "countdown.h"

#define DEFAUT_SERVER_PORT             5000
#define MAX_BUF                          64
#define CLIENT_QUEUE                     50
#define MAX_CLIENTS                       5
#define TASK_QUEUE                       80
#define MAX_WORKERS                      10
#define UNIX_SOCKET_PATH "/tmp/unix_socket"

#define CONN_WIDTH                       12
#define OP_WIDTH                         12
#define VEC_WIDTH                        15

typedef struct {
    int socketfd;
    threadpool_t *pool;
    threadpool_t *workers;
} server_args;

typedef struct {
    int socketfd;
    threadpool_t *pool;
    threadpool_t *workers;
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

typedef struct {
    int totalConnections;
    int totalOperations;
    int totalVectorSize;
} stats;

void process_client(int socketfd, threadpool_t *pool, threadpool_t *workers);
int vector_get_in_range_with_thread_pool (int v[], int v_sz, int sv[], int min, int max, threadpool_t *tp);
void* tcp_accept(void* args);
void* unix_accept(void* args);
void incrementConnection();
void incrementOperation();
void sumVectorSize(int size);
void* printStatistics(void* args);

stats totalStats;
pthread_mutex_t statsMutex;

void range_child(thread_args* args) {
    incrementOperation();

    for (int j = args->index * args->size; j < (args->index + 1) * args->size; j++) {
        if (args->v[j] >= args->min && args->v[j] <= args->max)
            args->res[args->res_count++] = args->v[j];
    }

    countdown_down(args->cd);
}

void* handle_client(void* args) {
    client_args *p_args = (client_args *)args;

    incrementConnection();
    process_client(p_args->socketfd, p_args->pool, p_args->workers);

    handle_error_system(close(p_args->socketfd), "[srv] closing socket to client");
    free(p_args);

    pthread_exit(NULL);
}

void process_client(int socketfd, threadpool_t *pool, threadpool_t *workers) {
    int buf[MAX_BUF];
    int cnt, min, max, nFullBatch, batchCount = 0, readBytes;

    readn(socketfd, buf, 3 * sizeof(int));
    cnt = buf[0];
    min = buf[1];
    max = buf[2];
    nFullBatch = cnt / MAX_BUF;

    sumVectorSize(cnt);

    int *vec = malloc(sizeof(int) * cnt), vecSize;
    if (vec == NULL) {
        handle_error_system(close(socketfd), "[srv] closing socket to client");
        fprintf(stderr, "Erro malloc\n");
        threadpool_destroy(pool);
        threadpool_destroy(workers);
        exit(EXIT_FAILURE);
    }

    int *allVals = malloc(sizeof(int) * cnt), allValsSize = 0;
    if (allVals == NULL) {
        handle_error_system(close(socketfd), "[srv] closing socket to client");
        fprintf(stderr, "Erro malloc\n");
        threadpool_destroy(pool);
        threadpool_destroy(workers);
        exit(EXIT_FAILURE);
    }

    while (cnt != allValsSize && (readBytes = readn(socketfd, buf, (batchCount < nFullBatch ? MAX_BUF : cnt % MAX_BUF) * sizeof(int))) > 0) {
        for (int i = 0; i < readBytes / sizeof(int); i++)
            allVals[allValsSize++] = buf[i];

        batchCount++;
    }

    vecSize = vector_get_in_range_with_thread_pool(allVals, cnt, vec, min, max, pool);
    if (vecSize == -1) {
        threadpool_destroy(workers);
        exit(EXIT_FAILURE);
    }

    handle_error_system(writen(socketfd, &vecSize, sizeof(int)), "Writing to client");
    handle_error_system(writen(socketfd, vec, sizeof(int) * vecSize), "Writing to client");

    free(allVals);
    free(vec);
}

int vector_get_in_range_with_thread_pool (int v[], int v_sz, int sv[], int min, int max, threadpool_t *tp) {
    thread_args targs[MAX_WORKERS];
    countdown_t cd;
    int num_elements = 0;
    int subarray_size = v_sz / MAX_WORKERS;

    countdown_init(&cd, MAX_WORKERS);

    for (int i = 0; i < MAX_WORKERS; i++) {
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

    for (int i = 0; i < MAX_WORKERS; i++) {
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

    pthread_t tcpThread, unixThread, statsThread;
    threadpool_t pool, workers;
    threadpool_init(&pool, CLIENT_QUEUE, MAX_CLIENTS);
    threadpool_init(&workers, TASK_QUEUE, MAX_WORKERS);

    printf("Server running on port %d\n", serverPort);

    int tcpSocketfd = tcp_socket_server_init(serverPort);
    handle_error_system(tcpSocketfd, "[srv] TCP server socket init");

    /*int unixSocketfd = un_socket_server_init(UNIX_SOCKET_PATH);
    handle_error_system(unixSocketfd, "[srv] UNIX server socket init");*/

    server_args tcpArgs = { tcpSocketfd, &pool, &workers };
    //server_args unixArgs = { unixSocketfd, &pool, &workers };
    pthread_mutex_init(&statsMutex, NULL);

    // TODO: remove

    if (pthread_create(&tcpThread, NULL, tcp_accept, &tcpArgs) != 0) {
        fprintf(stderr, "Error creating tcp thread\n");
        threadpool_destroy(&pool);
        threadpool_destroy(&workers);
        exit(EXIT_FAILURE);
    }

    /*if (pthread_create(&unixThread, NULL, unix_accept, &unixArgs) != 0) {
        fprintf(stderr, "Error creating unix thread\n");
        threadpool_destroy(&pool);
        threadpool_destroy(&workers);
        exit(EXIT_FAILURE);
    }*/

    if (pthread_create(&statsThread, NULL, printStatistics, NULL) != 0) {
        fprintf(stderr, "Error creating statistics thread\n");
        threadpool_destroy(&pool);
        threadpool_destroy(&workers);
        exit(EXIT_FAILURE);
    }

    while (getchar() != 't');

    printf("Stopping server\n");

    threadpool_destroy(&pool);
    threadpool_destroy(&workers);
    pthread_cancel(tcpThread);
    //pthread_cancel(unixThread);
    pthread_cancel(statsThread);
    pthread_mutex_destroy(&statsMutex);
    close(tcpSocketfd);
    //close(unixSocketfd);

    return 0;
}

void* tcp_accept(void* args) {
    server_args *s_args = (server_args *)args;

    while (1) {
        int newsocketfd = tcp_socket_server_accept(s_args->socketfd);

        handle_error_system(newsocketfd, "[srv] Accept new connection");

        client_args *p_args = malloc(sizeof(client_args));
        p_args->socketfd = newsocketfd;
        p_args->pool = s_args->pool;
        p_args->workers = s_args->workers;

        if (threadpool_submit(s_args->pool, handle_client, p_args) != 0) {
            handle_error_system(close(newsocketfd), "[srv] closing socket to client");
            fprintf(stderr, "Error submitting task\n");
            threadpool_destroy(s_args->pool);
            threadpool_destroy(s_args->workers);
            exit(EXIT_FAILURE);
        }
    }

    pthread_exit(NULL);
}

void* unix_accept(void* args) {
    server_args *s_args = (server_args *)args;

    while (1) {
        int newsocketfd = un_socket_server_accept(s_args->socketfd);

        handle_error_system(newsocketfd, "[srv] Accept new connection");

        client_args *p_args = malloc(sizeof(client_args));
        p_args->socketfd = newsocketfd;
        p_args->pool = s_args->pool;
        p_args->workers = s_args->workers;

        if (threadpool_submit(s_args->pool, handle_client, p_args) != 0) {
            handle_error_system(close(newsocketfd), "[srv] closing socket to client");
            fprintf(stderr, "Error submitting task\n");
            threadpool_destroy(s_args->pool);
            threadpool_destroy(s_args->workers);
            exit(EXIT_FAILURE);
        }
    }

    pthread_exit(NULL);
}

void incrementConnection() {
    pthread_mutex_lock(&statsMutex);
    totalStats.totalConnections++;
    pthread_mutex_unlock(&statsMutex);
}

void incrementOperation() {
    pthread_mutex_lock(&statsMutex);
    totalStats.totalOperations++;
    pthread_mutex_unlock(&statsMutex);
}

void sumVectorSize(int size) {
    pthread_mutex_lock(&statsMutex);
    totalStats.totalVectorSize += size;
    pthread_mutex_unlock(&statsMutex);
}

void* printStatistics(void* args) {
    printf("%-*s %-*s %-*s\n", CONN_WIDTH, "Connections", OP_WIDTH, "Operations", VEC_WIDTH, "Vector Size");
    printf("%-*s %-*s %-*s\n", CONN_WIDTH, "------------", OP_WIDTH, "------------", VEC_WIDTH, "---------------");

    while (1) {
        printf("%-*d %-*d %-*.2f\n", CONN_WIDTH, totalStats.totalConnections, OP_WIDTH, totalStats.totalOperations, VEC_WIDTH, (float)totalStats.totalVectorSize / (totalStats.totalOperations == 0 ? 1 : totalStats.totalOperations));
        sleep(1);
    }

    pthread_exit(NULL);
}