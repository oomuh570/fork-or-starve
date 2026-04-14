/*
 * File: philosopher.c
 * Purpose: Runs the philosopher thread loop for thinking, getting forks, eating,
 *          and checking starvation. Tracks fork ownership to detect conflicts.
 */

#include "philosopher.h"
#include "forks.h"
#include "semaphore.h"
#include "display.h"
#include "stats.h"
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

/*
 * Function: rand_sleep
 * Purpose:  Sleeps for a random duration between 100ms and 600ms.
 *           Used for thinking and eating to make scheduling more realistic.
 * Params:   none
 * Returns:  void
 */
void rand_sleep() {
    int low  = 100000;   /* 100ms */
    int high = 600000;   /* 600ms */
    usleep((rand() % (high - low + 1)) + low);
}

/*
 * Function: claim_fork
 * Purpose:  Claims a fork for a philosopher and logs it to simulation.log.
 *           Checks fork_holder to confirm no two philosophers hold the same fork.
 *           If a conflict is detected it prints a CONFLICT WARNING instead of crashing.
 * Params:
 *   fork_index - index of the fork being claimed
 *   phil_id    - philosopher number claiming the fork
 * Returns: void
 */
static void claim_fork(int fork_index, long phil_id) {
    sem_wait(&mutex);

    /* fork conflict detection — check if already held */
    if (fork_holder[fork_index] != -1) {
        fprintf(stderr,
            "*** FORK CONFLICT: Fork %d claimed by P%ld %s but already held by P%d %s ***\n",
            fork_index,
            phil_id, phil_names[phil_id],
            fork_holder[fork_index], phil_names[fork_holder[fork_index]]);
    } else {
        fork_holder[fork_index] = (int)phil_id;
    }

    sem_post(&mutex);

    /* log fork claim */
    FILE *log = fopen("simulation.log", "a");
    if (log) {
        fprintf(log, "P%ld %-12s -> FORK %d CLAIMED\n",
                phil_id, phil_names[phil_id], fork_index);
        fclose(log);
    }
}

/*
 * Function: release_fork
 * Purpose:  Releases a fork and logs it to simulation.log.
 *           Clears the fork_holder entry so the next philosopher can claim it.
 * Params:
 *   fork_index - index of the fork being released
 * Returns: void
 */
static void release_fork(int fork_index) {
    sem_wait(&mutex);

    /* log before clearing so we know who released it */
    FILE *log = fopen("simulation.log", "a");
    if (log) {
        fprintf(log, "P%d %-12s -> FORK %d RELEASED\n",
                fork_holder[fork_index],
                phil_names[fork_holder[fork_index]],
                fork_index);
        fclose(log);
    }

    fork_holder[fork_index] = -1;   /* mark fork as free */
    sem_post(&mutex);
}

/*
 * Function: log_state
 * Purpose:  Appends a philosopher state change to simulation.log.
 * Params:
 *   i         - philosopher number
 *   state_str - state name ("THINKING", "HUNGRY", "EATING")
 * Returns: void
 */
static void log_state(long i, const char *state_str) {
    FILE *log = fopen("simulation.log", "a");
    if (log) {
        fprintf(log, "P%ld %-12s -> %s\n", i, phil_names[i], state_str);
        fclose(log);
    }
}

/*
 * Function: think_and_eat
 * Purpose: Runs one philosopher's main loop. The philosopher thinks, becomes hungry,
 *          picks up forks based on the current mode, eats, puts forks down, and repeats.
 * Params:
 *   arg - philosopher number passed into the thread
 * Returns: void pointer
 */
void *think_and_eat(void *arg)
{
    long i = (long)arg;
    int count = 0;

    while (steps == -1 || count < steps) {

        /* THINKING */
        sem_wait(&mutex);
        state[i] = THINKING;
        sem_post(&mutex);
        if (!isatty(fileno(stdout)))
            printf("P%ld %s is THINKING\n", i, phil_names[i]);
        log_state(i, "THINKING");
        rand_sleep();

        /* HUNGRY */
        sem_wait(&mutex);
        state[i] = HUNGRY;
        sem_post(&mutex);
        if (!isatty(fileno(stdout)))
            printf("P%ld %s is HUNGRY\n", i, phil_names[i]);
        log_state(i, "HUNGRY");

        /* FORK PICKUP — depends on mode */

        /* MODE 2 - WAITER SOLUTION */
        if (mode == 2) {
            sem_wait(&waiter);   /* take a seat — blocks if num_waiters already seated */
            pthread_mutex_lock(&forks[left_fork(i)]);
            claim_fork(left_fork(i), i);
            pthread_mutex_lock(&forks[right_fork(i)]);
            claim_fork(right_fork(i), i);
        }
        /* MODE 1 - ASYMMETRIC — P4 picks up right fork first */ 
	else if (mode == 1) {
		int right_first = 0;

		if (asy_mode == ASY_ODD && (i % 2 != 0))
			right_first = 1;
		else if (asy_mode == ASY_EVEN && (i % 2 == 0))
			right_first = 0;

		if (right_first) {
			pthread_mutex_lock(&forks[right_fork(i)]);
			pthread_mutex_lock(&forks[left_fork(i)]);
		}
		else {
			pthread_mutex_lock(&forks[left_fork(i)]);
			pthread_mutex_lock(&forks[right_fork(i)]);
		}
	}

        /* MODE 1 - ASYMMETRIC — P4 picks up right fork first */
        else if (mode == 1 && i == NUM_PHILS - 1) {
            pthread_mutex_lock(&forks[right_fork(i)]);
            claim_fork(right_fork(i), i);
            rand_sleep();
            pthread_mutex_lock(&forks[left_fork(i)]);
            claim_fork(left_fork(i), i);
        }
        /* MODE 0 - NAIVE and MODE 1 everyone else — left first */
        else if (asy_mode == ASY_ODD) {
            pthread_mutex_lock(&forks[left_fork(i)]);
            pthread_mutex_lock(&forks[right_fork(i)]);
            claim_fork(right_fork(i), i);
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
        if (!isatty(fileno(stdout)))
            printf("P%ld %s is EATING\n", i, phil_names[i]);
        log_state(i, "EATING");
        rand_sleep();

        /* PUT FORKS DOWN — release and unlock */
        release_fork(right_fork(i));
        pthread_mutex_unlock(&forks[right_fork(i)]);
        release_fork(left_fork(i));
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

