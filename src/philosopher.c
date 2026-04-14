/*
 * File: philosopher.c
 * Purpose: Runs the philosopher thread loop for thinking, getting forks, eating, and checking starvation.
 */
#include "philosopher.h"
#include "forks.h"
#include "semaphore.h"
#include "display.h"
#include "stats.h"
#include <unistd.h>
#include <stdio.h>

// for random sleep
#include <stdlib.h>
#include <time.h>

/*
 * Function: think_and_eat
 * Purpose: Runs one philosopher's main loop. The philosopher thinks, becomes hungry,
 *          picks up forks based on the current mode, eats, puts forks down, and repeats.
 * Params:
 *   arg - philosopher number passed into the thread
 * Returns: void pointer
 */
void *think_and_eat(void *arg)     /* executed concurrently by all philosophers */
{
	srand(time(NULL));
    long i = (long)arg;
    int count = 0;
    
    while (steps == -1 || count < steps) {

        /* THINKING */
        sem_wait(&mutex);
        state[i] = THINKING;
        sem_post(&mutex);
        /* print plain text when output redirected to file — for testing */
        if (!isatty(fileno(stdout)))
            printf("P%ld %s is THINKING\n", i, phil_names[i]);
        //usleep(500000);
		rand_sleep();

        /* HUNGRY */
        sem_wait(&mutex);
        state[i] = HUNGRY;
        sem_post(&mutex);
        /* print plain text when output redirected to file — for testing */
        if (!isatty(fileno(stdout)))
            printf("P%ld %s is HUNGRY\n", i, phil_names[i]);

        /* FORK PICKUP — depends on mode */

        /* MODE 2 - WAITER SOLUTION */
        if (mode == 2) {
            sem_wait(&waiter);   /* take a seat — blocks if 4 already seated */
            pthread_mutex_lock(&forks[left_fork(i)]);
            pthread_mutex_lock(&forks[right_fork(i)]);
        }
        /* MODE 1 - ASYMMETRIC — P4 picks up right fork first */ 
	else if (mode == 1) {
		int right_first = 0;

		if (asy_mode == ASY_ODD && (i % 2 != 0))
			right_first = 1;
		else if (asy_mode == ASY_EVEN && (i % 2 == 0))
			right_first == 1;

		if (right_first) {
			pthread_mutex_lock(&forks[right_fork(i)]);
        	    	usleep(500000);
			pthread_mutex_lock(&forks[left_fork(i)]);
		}
		else {
			pthread_mutex_lock(&forks[left_fork(i)]);
			usleep(500000);
			pthread_mutex_lock(&forks[right_fork(i)]);
		}

        /* MODE 1 - ASYMMETRIC — P4 picks up right fork first */
        else if (mode == 1 && i == NUM_PHILS - 1) {
            pthread_mutex_lock(&forks[right_fork(i)]);
            //usleep(500000);
			rand_sleep();
            pthread_mutex_lock(&forks[left_fork(i)]);
        }
        /* MODE 0 - NAIVE and MODE 1 everyone else — left first */
        else if (asy_mode == ASY_ODD) {
            pthread_mutex_lock(&forks[left_fork(i)]);
            pthread_mutex_lock(&forks[right_fork(i)]);
        }

	else if (asy_mode == ASY_EVEN) {
		pthread_mutex_lock(&forks[right_fork(i)]);
		pthread_mutex_lock(&forks[left_fork(i)]);
	}

        /* EATING */
        sem_wait(&mutex);
        state[i] = EATING;
        meal_record(i);
        sem_post(&mutex);
        /* print plain text when output redirected to file — for testing */
        if (!isatty(fileno(stdout)))
            printf("P%ld %s is EATING\n", i, phil_names[i]);
        //usleep(500000);
		rand_sleep();

        /* PUT FORKS DOWN */
        pthread_mutex_unlock(&forks[right_fork(i)]);
        pthread_mutex_unlock(&forks[left_fork(i)]);

        /* release waiter seat after putting forks down */
        if (mode == 2) {
            sem_post(&waiter);
        }

        sem_wait(&mutex);
        starve_check(count);
        sem_post(&mutex);

        count++;
    }

    return 0;
}

void rand_sleep() {
	int high = 500000;
	int low = 50;
	int wait = (rand() % (high - low + 1)) + low;
	usleep(wait);
}
