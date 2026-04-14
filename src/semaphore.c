/*
 * File: semaphore.c
 * Purpose: Sets up the semaphores used in the program.
 */

#include "semaphore.h"

sem_t mutex;
sem_t waiter;

/*
 * Function: init_semaphores
 * Purpose: Initializes the mutex semaphore and waiter semaphore.
 * Params: none
 * Returns: void
 */
void init_semaphores(int waiters) {
    sem_init(&mutex, 0, 1);
    sem_init(&waiter, 0, waiters);
}
