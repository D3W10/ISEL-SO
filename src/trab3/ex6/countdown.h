#include <pthread.h>

#ifndef COUNTDOWN_H
#define COUNTDOWN_H

typedef struct {
    int count;
    pthread_cond_t cond;
    pthread_mutex_t mutex;
} countdown_t;

int countdown_init(countdown_t *cd, int initialValue);
int countdown_destroy(countdown_t *cd);
int countdown_wait(countdown_t *cd);
int countdown_down(countdown_t *cd);

#endif