/*
 * File: semaphore.h
 * Purpose: Header file for semaphore variables and functions.
 */

#ifndef SEMAPHORE_H
#define SEMAPHORE_H

#include <semaphore.h>

extern sem_t mutex;
extern sem_t waiter;

/*
 * Function: init_semaphores
 * Purpose: Initializes the semaphores used in the program.
 * Params: none
 * Returns: void
 */
void init_semaphores(int waiters);

#endif
