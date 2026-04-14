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

/* number of failed trylock attempts before giving up and releasing first fork */
#define MAX_ATTEMPTS 1

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
 * Function: trylock_fork
 * Purpose:  Tries to acquire a fork using trylock with backoff — no infinite waiting.
 *           If the fork cannot be acquired after MAX_ATTEMPTS tries and a first fork
 *           is already held, releases the first fork and logs GAVE UP.
 *           Only used for mode=1 and mode=2 — mode=0 uses pthread_mutex_lock
 *           to preserve deadlock demonstration.
 * Params:
 *   fork_index - index of the fork to acquire
 *   phil_id    - philosopher number
 *   first_fork - index of first fork already held (-1 if acquiring first fork)
 * Returns: 1 if acquired, 0 if gave up and released first fork
 */
static int trylock_fork(int fork_index, long phil_id, int first_fork) {
    int attempts = 0;

    while (pthread_mutex_trylock(&forks[fork_index]) != 0) {
        attempts++;

        /* gave up — release first fork and start over */
        if (attempts >= MAX_ATTEMPTS && first_fork != -1) {

            /* print to terminal when redirected */
            if (!isatty(fileno(stdout)))
                printf("P%ld %s gave up waiting for Fork %d — releasing Fork %d and retrying\n",
                       phil_id, phil_names[phil_id], fork_index, first_fork);

            /* log the gave up event */
            FILE *log = fopen("simulation.log", "a");
            if (log) {
                fprintf(log, "P%ld %-12s -> GAVE UP Fork %d — releasing Fork %d retrying\n",
                        phil_id, phil_names[phil_id], fork_index, first_fork);
                fclose(log);
            }

            /* release the first fork */
            release_fork(first_fork);
            pthread_mutex_unlock(&forks[first_fork]);

            rand_sleep();   /* wait before trying again */
            return 0;       /* signal caller to retry from scratch */
        }

        usleep((rand() % 50 + 10) * 1000);   /* back off 10-60ms and retry */
    }

    /* successfully acquired — record and log */
    claim_fork(fork_index, phil_id);
    return 1;
}

/*
 * Function: pickup_forks_safe
 * Purpose:  Picks up both forks using trylock with backoff and gave up retry.
 *           If second fork times out releases first and retries everything.
 *           Used for mode=1 and mode=2 only.
 * Params:
 *   i          - philosopher number
 *   first      - index of first fork to pick up
 *   second     - index of second fork to pick up
 * Returns: void
 */
static void pickup_forks_safe(long i, int first, int second) {
    while (1) {
        /* acquire first fork — no first fork held yet so pass -1 */
        while (trylock_fork(first, i, -1) == 0) { /* retry until acquired */ }

        /* acquire second fork — pass first so it can be released on timeout */
        if (trylock_fork(second, i, first) == 1) {
            break;   /* got both forks */
        }
        /* gave up — first fork released — retry everything */
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

        /* MODE 2 - WAITER SOLUTION — safe trylock pickup */
        if (mode == 2) {
            sem_wait(&waiter);   /* take a seat — blocks if num_waiters already seated */
            pickup_forks_safe(i, left_fork(i), right_fork(i));
        }
        /* MODE 1 - ASYMMETRIC — safe trylock pickup with odd/even */
        else if (mode == 1) {
            int right_first = 0;

            if (asy_mode == ASY_ODD && (i % 2 != 0))
                right_first = 1;
            else if (asy_mode == ASY_EVEN && (i % 2 == 0))
                right_first = 1;

            if (right_first) {
                pickup_forks_safe(i, right_fork(i), left_fork(i));
            } else {
                pickup_forks_safe(i, left_fork(i), right_fork(i));
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
        /* MODE 0 - NAIVE — plain mutex_lock to preserve deadlock demonstration */
        else {
            pthread_mutex_lock(&forks[left_fork(i)]);
            claim_fork(left_fork(i), i);
            pthread_mutex_lock(&forks[right_fork(i)]);
            claim_fork(right_fork(i), i);
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

