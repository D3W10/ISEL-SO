#include <stdio.h>
#include <pthread.h>

void *th1(void *arg) {
    int *pt = (int *)arg;

    for (int i = 0; i < 10000000; ++i)
        (*pt) += 2;

    return NULL;
}

void *th2(void *arg) {
    int *pt = (int *)arg;

    for (int i = 0; i < 10000000; ++i)
        (*pt) -= 3;

    return NULL;
}

void *th3(void *arg) {
    int *pt = (int *)arg;

    for (int i = 0; i < 10000000; ++i)
        (*pt)++;

    return NULL;
}

int main() {
    int count = 0;
    pthread_t t1, t2, t3;

    pthread_create(&t1, NULL, th1, &count);
    pthread_create(&t2, NULL, th2, &count);
    pthread_create(&t3, NULL, th3, &count);
    pthread_join(t1, NULL);
    pthread_join(t2, NULL);
    pthread_join(t3, NULL);

    printf("Total = %d\n", count);
}