#include <stdio.h>
#include <stdlib.h>
#include "thread_pool.h"

void *thread_do_work(void *threadpool);

int threadpool_init(threadpool_t *tp, int queueDim, int nthreads) {
    if (tp == NULL || queueDim <= 0 || nthreads <= 0)
        return -1;

    tp->queue_max_size = queueDim;
    tp->thread_count = nthreads;
    tp->queue_size = 0;
    tp->shutdown = 0;
    tp->queue_head = tp->queue_tail = NULL;

    tp->threads = (pthread_t *)malloc(sizeof(pthread_t) * nthreads);
    if (tp->threads == NULL) {
        fprintf(stderr, "Error allocating memory for threads\n");
        return -1;
    }

    if (pthread_mutex_init(&(tp->lock), NULL) != 0 || pthread_cond_init(&(tp->notify), NULL) != 0) {
        fprintf(stderr, "Error initializing mutex or condition variable\n");
        free(tp->threads);
        return -1;
    }

    for (int i = 0; i < nthreads; i++) {
        if (pthread_create(&(tp->threads[i]), NULL, thread_do_work, (void *)tp) != 0) {
            fprintf(stderr, "Error creating thread %d\n", i);
            threadpool_destroy(tp);
            return -1;
        }
    }

    return 0;
}

int threadpool_submit(threadpool_t *tp, wi_function_t func, void *arg) {
    if (tp == NULL || func == NULL || tp->shutdown)
        return -1;

    task_t *task = (task_t *)malloc(sizeof(task_t));
    if (task == NULL) {
        fprintf(stderr, "Error allocating memory for task\n");
        return -1;
    }

    task->function = func;
    task->arg = arg;
    task->next = NULL;

    pthread_mutex_lock(&(tp->lock));

    if (tp->queue_size >= tp->queue_max_size) {
        free(task);
        pthread_mutex_unlock(&(tp->lock));
        fprintf(stderr, "Error: queue is full\n");
        return -1;
    }

    if (tp->queue_tail == NULL) {
        tp->queue_head = task;
        tp->queue_tail = task;
    }
    else {
        tp->queue_tail->next = task;
        tp->queue_tail = task;
    }
    tp->queue_size++;

    pthread_cond_signal(&(tp->notify));
    pthread_mutex_unlock(&(tp->lock));

    return 0;
}

int threadpool_destroy(threadpool_t *tp) {
    if (tp == NULL)
        return -1;

    pthread_mutex_lock(&(tp->lock));
    tp->shutdown = 1;
    pthread_cond_broadcast(&(tp->notify));
    pthread_mutex_unlock(&(tp->lock));

    for (int i = 0; i < tp->thread_count; i++)
        pthread_join(tp->threads[i], NULL);

    free(tp->threads);

    while (tp->queue_head != NULL) {
        task_t *task = tp->queue_head;
        tp->queue_head = tp->queue_head->next;
        free(task);
    }

    pthread_mutex_destroy(&(tp->lock));
    pthread_cond_destroy(&(tp->notify));

    return 0;
}

void *thread_do_work(void *threadpool) {
    threadpool_t *tp = (threadpool_t *)threadpool;

    while (1) {
        pthread_mutex_lock(&(tp->lock));

        while (tp->queue_size == 0 && !tp->shutdown)
            pthread_cond_wait(&(tp->notify), &(tp->lock));

        if (tp->queue_size == 0 && tp->shutdown) {
            pthread_mutex_unlock(&(tp->lock));
            pthread_exit(NULL);
        }

        task_t *task = tp->queue_head;
        if (tp->queue_head == tp->queue_tail) {
            tp->queue_head = NULL;
            tp->queue_tail = NULL;
        }
        else
            tp->queue_head = task->next;

        tp->queue_size--;

        pthread_mutex_unlock(&(tp->lock));

        (*(task->function))(task->arg);
        free(task);
    }

    pthread_exit(NULL);
    return NULL;
}