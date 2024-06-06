#include <pthread.h>

#ifndef THREAD_POOL_H
#define THREAD_POOL_H

typedef void *(*wi_function_t)(void *);

typedef struct task {
    wi_function_t function;
    void *arg;
    struct task *next;
} task_t;

typedef struct {
    pthread_mutex_t lock;
    pthread_cond_t notify;
    pthread_t *threads;
    task_t *queue_head;
    task_t *queue_tail;
    int queue_size;
    int queue_max_size;
    int thread_count;
    int shutdown;
} threadpool_t;

int threadpool_init(threadpool_t *tp, int queueDim, int nthreads);
int threadpool_submit(threadpool_t *tp, wi_function_t func, void *args);
int threadpool_destroy(threadpool_t *tp);

#endif