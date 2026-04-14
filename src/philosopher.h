/*
 * File: philosopher.h
 * Purpose: Header file for philosopher threads and shared philosopher data.
 */

#ifndef PHILOSOPHER_H
#define PHILOSOPHER_H

#include <pthread.h>

#define NUM_PHILS 5

#define THINKING 0
#define HUNGRY   1
#define EATING   2

extern pthread_mutex_t forks[NUM_PHILS];
extern int   state[NUM_PHILS];
extern char *phil_names[NUM_PHILS];
extern int   meals[NUM_PHILS];
extern int   fork_holder[NUM_PHILS];   /* tracks which philosopher holds each fork — -1 if free */

extern int mode;
extern int steps;

/*
 * Function: think_and_eat
 * Purpose: Runs the main loop for one philosopher thread.
 * Params:
 *   arg - philosopher number passed into the thread
 * Returns: void pointer
 */
void *think_and_eat(void *);

/*
 * Function: rand_sleep
 * Purpose: Sleeps for a random duration between 100ms and 600ms.
 * Returns: void
 */
void rand_sleep();

#endif