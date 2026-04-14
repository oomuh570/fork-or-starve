#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#include "philosopher.h"
#include "semaphore.h"
#include "display.h"
#include "stats.h"

pthread_mutex_t forks[NUM_PHILS];
int   state[NUM_PHILS];
char *phil_names[NUM_PHILS] = {"Aristotle", "Plato", "Socrates", "Descartes", "Kant"};
int   meals[NUM_PHILS]      = {0};
int   fork_holder[NUM_PHILS];   /* -1 = free, N = held by philosopher N */

int asy_mode = ASY_NONE;
int waiter_num = 4;

int main(int argc, char *argv[])
{
    pthread_t philosopher[NUM_PHILS];
    pthread_t display;
    long i;

    for (int i = 1; i < argc; i++) {
        if (strncmp(argv[i], "--mode=", 7) == 0) {
            mode = atoi(argv[i] + 7);
        }
        else if (strncmp(argv[i], "--steps=", 8) == 0) {
            steps = atoi(argv[i] + 8);
        }
	else if (strncmp (argv[i], "--asy=", 6) == 0) {
		if (strcmp (argv[i] + 6,  "odd") == 0) {
			asy_mode = ASY_ODD;
		}
		else if (strcmp (argv[i] + 6, "even") == 0) {
			asy_mode = ASY_EVEN;
		}
		else {
			printf("Invalid asymmetric mode selected.");
			exit(1);
		}
	}
		else if (strncmp(argv[i], "--waiters=", 10) == 0) { // New argument check
        waiter_num = atoi(argv[i] + 10);
		}
        else if (strcmp(argv[i], "--help") == 0) {
            printf("\nUsage: ./dining_philosophers [OPTIONS]\n\n");
            printf("  --mode=0     Naive — deadlock will occur\n");
            printf("  --mode=1     Asymmetric — no deadlock\n");
            printf("  --mode=2     Waiter semaphore — no deadlock\n");
			printf("  --waiters=N  Number of waiters serving the philosopher\n");
            printf("  --steps=N    Number of cycles per philosopher\n");
            printf("  --help       Show this help message\n\n");
		printf("  --asy=odd Odd philosophers pick up right fork first\n");
		printf("   --asy-even Even philosophers pick up right fork first\n");
            exit(0);
        }
        else {
            printf("Unknown argument \"%s\" detected\n", argv[i]);
            exit(1);
        }
    }

	if (asy_mode != ASY_NONE && mode !=1) {
		printf("Error: --asy can only be used with --mode=1\n");
		exit(1);
	}

	if (mode == 1 && asy_mode == ASY_NONE) {
		asy_mode = ASY_ODD;
	}

    if (mode != 0 && mode != 1 && mode != 2) {
        printf("Invalid mode selected, exiting program\n");
        exit(1);
    }

    if (steps != -1) {
        printf("Running mode=%d for %d steps | waiters=%d\n", mode, steps, waiter_num);
    }
    else {
        printf("Running mode=%d | waiters=%d\n", mode, waiter_num);
    }

    srand(time(0));   /* seed once in main — not in threads */
    init_semaphores(waiter_num);

    /* initialize forks, states, and fork_holder */
    for (i = 0; i < NUM_PHILS; i++) {
        pthread_mutex_init(&forks[i], NULL);
        state[i]       = THINKING;
        fork_holder[i] = -1;   /* -1 = fork is free */
    }

    /* clear simulation log at start of each run */
    FILE *log = fopen("simulation.log", "w");
    if (log) fclose(log);

    /* only start display thread if running in a real terminal */
    if (isatty(fileno(stdout))) {
        pthread_create(&display, NULL, display_thread, NULL);
    }

    /* create philosopher threads */
    for (i = 0; i < NUM_PHILS; i++)
        pthread_create(philosopher + i, 0, think_and_eat, (void *)(long)i);

    /* join philosopher threads */
    for (i = 0; i < NUM_PHILS; i++)
        pthread_join(philosopher[i], NULL);

    /* print final statistics */
    print_stats();
    fflush(stdout);

    return 0;
}
