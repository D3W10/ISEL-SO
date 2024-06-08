#include <stdio.h>
#include <pthread.h>

/**
 * Este programa produz sempre o mesmo resultado pois cada thread calcula a sua soma separadamente e quando todas as threads terminarem, os resultados das somas são somados no count final.
 */

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
    int a = 0, b = 0, c = 0;

    pthread_create(&t1, NULL, th1, &a);
    pthread_create(&t2, NULL, th2, &b);
    pthread_create(&t3, NULL, th3, &c);
    pthread_join(t1, NULL);
    pthread_join(t2, NULL);
    pthread_join(t3, NULL);

    count = a + b + c;
    printf("Total = %d\n", count);
}