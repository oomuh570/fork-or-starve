#ifndef PHILOSOPHER_H
#define PHILOSOPHER_H

#include <pthread.h>

#define NUM_PHILS 5

#define THINKING 0
#define HUNGRY 1
#define EATING 2

extern pthread_mutex_t forks[NUM_PHILS];
extern int state[NUM_PHILS];
extern char *phil_names[NUM_PHILS];
extern int meals[NUM_PHILS];

extern int mode;
extern int steps;

void *think_and_eat(void *);

#endif

