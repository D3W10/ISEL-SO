#include <stdio.h>
#include <pthread.h>

/**
 * Este programa produz sempre o mesmo resultado pois o valor da variável count é controlado através do mutex, permitindo acesso único ao count.
 */

pthread_mutex_t mutex;

void *th1(void *arg) {
    int *pt = (int *)arg;

    pthread_mutex_lock(&mutex);

    for (int i = 0; i < 10000000; ++i)
        (*pt) += 2;

    pthread_mutex_unlock(&mutex);

    return NULL;
}

void *th2(void *arg) {
    int *pt = (int *)arg;

    pthread_mutex_lock(&mutex);

    for (int i = 0; i < 10000000; ++i)
        (*pt) -= 3;

    pthread_mutex_unlock(&mutex);

    return NULL;
}

void *th3(void *arg) {
    int *pt = (int *)arg;

    pthread_mutex_lock(&mutex);

    for (int i = 0; i < 10000000; ++i)
        (*pt)++;

    pthread_mutex_unlock(&mutex);

    return NULL;
}

int main() {
    int count = 0;
    pthread_t t1, t2, t3;
    pthread_mutex_init(&mutex, NULL);

    pthread_create(&t1, NULL, th1, &count);
    pthread_create(&t2, NULL, th2, &count);
    pthread_create(&t3, NULL, th3, &count);
    pthread_join(t1, NULL);
    pthread_join(t2, NULL);
    pthread_join(t3, NULL);

    pthread_mutex_destroy(&mutex);

    printf("Total = %d\n", count);
}