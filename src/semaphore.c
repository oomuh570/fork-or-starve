#include "semaphore.h"

sem_t mutex;
sem_t waiter;

void init_semaphores() {
	sem_init(&mutex, 0, 1);
	sem_init(&waiter, 0, 4);
}
