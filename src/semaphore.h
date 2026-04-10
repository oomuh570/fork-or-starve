#ifndef SEMAPHORE_H
#define SEMAPHORE_H

#include <semaphore.h>

extern sem_t mutex;
extern sem_t waiter;

void init_semaphores();

#endif
