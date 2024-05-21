#include <pthread.h>
#include <semaphore.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#define MAX_CAPACITY 10

typedef struct ChargePoint_t {
    char lugares[MAX_CAPACITY];
    sem_t sLugaresLivres;
    pthread_mutex_t mutex;
    int priority;
} ChargePoint;

int reserveChargePoint(ChargePoint *point) {
    int pos = -1;

    pthread_mutex_lock(&point->mutex);
    for (int i = 0; i < MAX_CAPACITY; i++) {
        sem_wait(&point->sLugaresLivres);

        if (point->lugares[i] == 1) {
            pos = i;
            point->lugares[i] = 0;
            break;
        }
    }
    
    pthread_mutex_unlock(&point->mutex);

    return pos;
}

void freeChargePoint(ChargePoint *point, int lugar) {
    pthread_mutex_lock(&point->mutex);
    point->lugares[lugar] = 1;
    pthread_mutex_unlock(&point->mutex);
    sem_post(&point->sLugaresLivres);
}

int reserveChargePointPriority(ChargePoint *point) {
    
    
    return ;
}