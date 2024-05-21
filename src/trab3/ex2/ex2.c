#include <pthread.h>
#include <semaphore.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#define MAX_CAPACITY 10

typedef struct ChargePoint_t {
    char lugares[MAX_CAPACITY];
    sem_t sLugaresLivres;
    sem_t sFilaPrioritaria;
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


    if(point->priority > 0){
        sem_post(&point->sFilaPrioritaria);
    }
}

int reserveChargePointPriority(ChargePoint *point) {
    int pos = -1;    

    pthread_mutex_lock(&point->mutex);
    point->priority++;
    pthread_mutex_unlock(&point->mutex);

    sem_wait(&point->sFilaPrioritaria);

    pthread_mutex_lock(&point->mutex);

    sem_wait(&point->sLugaresLivres);

    for(int i = 0; i < MAX_CAPACITY; i++){
        if(point->lugares[i] == 1){
            pos = i;
            point->lugares[i] = 0;
            break;
        }
    }
    
    point->priority--;

    if(point->priority > 0){
        sem_post(&point->sFilaPrioritaria);
    }

    pthread_mutex_unlock(&point->mutex);

    return pos;
}

int main() {
    ChargePoint chargePoint;
    sem_init (&chargePoint.sLugaresLivres, 0, MAX_CAPACITY);
    sem_init (&chargePoint.sFilaPrioritaria, 0, 0);
    pthread_mutex_init (&chargePoint.mutex, NULL);
    
    for(int i = 0; i < MAX_CAPACITY; i++){
        chargePoint.lugares[i] = 1;
    }


    
    sem_destroy (&chargePoint.sLugaresLivres);
    sem_destroy (&chargePoint.sFilaPrioritaria);
    pthread_mutex_destroy (&chargePoint.mutex);

    return 0;
}