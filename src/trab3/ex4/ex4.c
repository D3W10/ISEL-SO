#include <pthread.h>
#include <stdlib.h>
#include <stdio.h>

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

static void *thread_do_work(void *threadpool);
static void add_task_to_queue(threadpool_t *pool, task_t *task);
static task_t *remove_task_from_queue(threadpool_t *pool);

int threadpool_init(threadpool_t *pool, int queueDim, int nthreads) {
    if (pool == NULL || queueDim <= 0 || nthreads <= 0) return -1;

    pool->queue_max_size = queueDim;
    pool->thread_count = nthreads;
    pool->queue_size = 0;
    pool->shutdown = 0;
    pool->queue_head = pool->queue_tail = NULL;

    pool->threads = (pthread_t *)malloc(sizeof(pthread_t) * nthreads);
    if (pool->threads == NULL) return -1;

    if (pthread_mutex_init(&(pool->lock), NULL) != 0 ||
        pthread_cond_init(&(pool->notify), NULL) != 0) {
        free(pool->threads);
        return -1;
    }

    for (int i = 0; i < nthreads; i++) {
        if (pthread_create(&(pool->threads[i]), NULL, thread_do_work, (void *)pool) != 0) {
            threadpool_destroy(pool);
            return -1;
        }
    }

    return 0;
}

int threadpool_submit(threadpool_t *pool, wi_function_t func, void *arg) {
    if (pool == NULL || func == NULL) return -1;

    task_t *task = (task_t *)malloc(sizeof(task_t));
    if (task == NULL) return -1;

    task->function = func;
    task->arg = arg;
    task->next = NULL;

    pthread_mutex_lock(&(pool->lock));

    if (pool->queue_size >= pool->queue_max_size) {
        free(task);
        pthread_mutex_unlock(&(pool->lock));
        return -1;
    }

    add_task_to_queue(pool, task);
    pool->queue_size++;

    pthread_cond_signal(&(pool->notify));
    pthread_mutex_unlock(&(pool->lock));

    return 0;
}

int threadpool_destroy(threadpool_t *pool) {
    if (pool == NULL) return -1;

    pthread_mutex_lock(&(pool->lock));
    pool->shutdown = 1;
    pthread_cond_broadcast(&(pool->notify));
    pthread_mutex_unlock(&(pool->lock));

    for (int i = 0; i < pool->thread_count; i++) {
        pthread_join(pool->threads[i], NULL);
    }

    free(pool->threads);

    while (pool->queue_head != NULL) {
        task_t *task = pool->queue_head;
        pool->queue_head = pool->queue_head->next;
        free(task);
    }

    pthread_mutex_destroy(&(pool->lock));
    pthread_cond_destroy(&(pool->notify));

    return 0;
}

static void *thread_do_work(void *threadpool) {
    threadpool_t *pool = (threadpool_t *)threadpool;

    while (1) {
        pthread_mutex_lock(&(pool->lock));

        while (pool->queue_size == 0 && !pool->shutdown) {
            pthread_cond_wait(&(pool->notify), &(pool->lock));
        }

        if (pool->shutdown) {
            pthread_mutex_unlock(&(pool->lock));
            pthread_exit(NULL);
        }

        task_t *task = remove_task_from_queue(pool);
        pool->queue_size--;

        pthread_mutex_unlock(&(pool->lock));

        (*(task->function))(task->arg);
        free(task);
    }

    pthread_exit(NULL);
    return NULL;
}

static void add_task_to_queue(threadpool_t *pool, task_t *task) {
    if (pool->queue_tail == NULL) {
        pool->queue_head = task;
        pool->queue_tail = task;
    } else {
        pool->queue_tail->next = task;
        pool->queue_tail = task;
    }
}

static task_t *remove_task_from_queue(threadpool_t *pool) {
    task_t *task = pool->queue_head;
    if (pool->queue_head == pool->queue_tail) {
        pool->queue_head = NULL;
        pool->queue_tail = NULL;
    } else {
        pool->queue_head = task->next;
    }
    return task;
}

void *test_function(void *arg) {
    int num = *((int *)arg);
    printf("Thread %d is working\n", num);
    sleep(1);
    return NULL;
}

int main() {
    threadpool_t pool;
    threadpool_init(&pool, 10, 4);

    int args[10];
    for (int i = 0; i < 10; i++) {
        args[i] = i;
        threadpool_submit(&pool, test_function, &args[i]);
    }

    sleep(3);
    threadpool_destroy(&pool);

    return 0;
}
