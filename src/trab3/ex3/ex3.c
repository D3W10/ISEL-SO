#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

typedef struct {
    int count;
    pthread_cond_t cond;
    pthread_mutex_t mutex;
} countdown_t;


int countdown_init(countdown_t *cd, int initial_value) {
    if (initial_value <= 0) {
        fprintf(stderr, "Initial value can't be equal or lower than 0");
        exit(EXIT_FAILURE);
    }
    
    cd->count = initial_value;
    pthread_cond_init(&cd->cond, NULL);
    pthread_mutex_init(&cd->mutex, NULL);

    return 0;
}

int countdown_destroy(countdown_t *cd) {
    pthread_cond_destroy(&cd->cond);
    pthread_mutex_destroy(&cd->mutex);

    return 0;
}

int countdown_wait(countdown_t *cd) {
    pthread_mutex_lock(&cd->mutex);

    while (cd->count != 0) {
        pthread_cond_wait(&cd->cond, &cd->mutex);
    }
    
    pthread_mutex_unlock(&cd->mutex);

    return -1;
}

int countdown_down(countdown_t *cd) {
    pthread_mutex_lock(&cd->mutex);

    if (cd->count > 0) {
        cd->count--;
        if (cd->count == 0) {
            pthread_cond_broadcast(&cd->cond);
        }
    }

    pthread_mutex_unlock(&cd->mutex);

    return 1;
}