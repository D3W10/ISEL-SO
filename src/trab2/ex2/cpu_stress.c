#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <pthread.h>

#define MAX 50

void process_work(long niter) {
    for (long i = 0; i < niter; i++)
        sqrt(rand());

    pthread_exit(NULL);
}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        printf("Usage: %s <number_of_threads>\n", argv[0]);
        return -1;
    }

    int pNum = atoi(argv[1]);
    long n = 1e9;
    pthread_t th[MAX];

    for (int i = 0; i < pNum; i++) {
        pthread_create(&th[i], NULL, (void *(*)(void *))process_work, (void *)n);
        printf("Threaded %lu\n", th[i]);
    }

    for (int i = 0; i < pNum; i++)
        pthread_join(th[i], NULL);

    return 0;
}