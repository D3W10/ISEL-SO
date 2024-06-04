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

    sem_wait(&point->sLugaresLivres);
    pthread_mutex_lock(&point->mutex);

    for (int i = 0; i < MAX_CAPACITY; i++) {
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

    if (point->priority > 0)
        sem_post(&point->sFilaPrioritaria);
}

int reserveChargePointPriority(ChargePoint *point) {
    int pos = -1;

    pthread_mutex_lock(&point->mutex);
    point->priority++;
    pthread_mutex_unlock(&point->mutex);

    sem_wait(&point->sFilaPrioritaria);
    sem_wait(&point->sLugaresLivres);
    pthread_mutex_lock(&point->mutex);

    for (int i = 0; i < MAX_CAPACITY; i++) {
        if (point->lugares[i] == 1) {
            pos = i;
            point->lugares[i] = 0;
            break;
        }
    }

    point->priority--;

    if (point->priority > 0)
        sem_post(&point->sFilaPrioritaria);

    pthread_mutex_unlock(&point->mutex);

    return pos;
}

void initChargePoint(ChargePoint *point) {
    for (int i = 0; i < MAX_CAPACITY; i++) {
        point->lugares[i] = 1;
    }
    sem_init(&point->sLugaresLivres, 0, MAX_CAPACITY);
    sem_init(&point->sFilaPrioritaria, 0, 0);
    pthread_mutex_init(&point->mutex, NULL);
    point->priority = 0;
}

void *test_thread(void *arg) {
    ChargePoint *point = (ChargePoint *)arg;
    int pos = reserveChargePoint(point);
    if (pos != -1) {
        printf("Reserved spot: %d\n", pos);
        sleep(1); // Simulate some work
        freeChargePoint(point, pos);
        printf("Freed spot: %d\n", pos);
    } else {
        printf("No spots available\n");
    }
    return NULL;
}

void *test_thread_priority(void *arg) {
    ChargePoint *point = (ChargePoint *)arg;
    int pos = reserveChargePointPriority(point);
    if (pos != -1) {
        printf("Priority reserved spot: %d\n", pos);
        sleep(1); // Simulate some work
        freeChargePoint(point, pos);
        printf("Priority freed spot: %d\n", pos);
    } else {
        printf("No priority spots available\n");
    }
    return NULL;
}

int main() {
    ChargePoint chargePoint;
    initChargePoint(&chargePoint);

    pthread_t threads[20];

    for (int i = 0; i < 10; i++) {
        pthread_create(&threads[i], NULL, test_thread, (void *)&chargePoint);
    }

    // Create threads for priority reservation
    for (int i = 10; i < 20; i++) {
        pthread_create(&threads[i], NULL, test_thread_priority, (void *)&chargePoint);
    }

    // Join all threads
    for (int i = 0; i < 20; i++) {
        pthread_join(threads[i], NULL);
    }

    return 0;
}