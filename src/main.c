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
int state[NUM_PHILS];
char *phil_names[NUM_PHILS] = {"Aristotle", "Plato", "Socrates", "Descartes", "Kant"};
int meals[NUM_PHILS] = {0};

int mode = 0;
int steps = -1;

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
            exit(0);
        }
        else {
            printf("Unknown argument \"%s\" detected\n", argv[i]);
            exit(1);
        }
    }

    if (mode != 0 && mode != 1 && mode != 2) {
        printf("Invalid mode selected, exiting program\n");
        exit(1);
    }

    if (steps != -1) {
        printf("Running mode=%d for %d steps\n", mode, steps);
    }
    else {
        printf("Running mode=%d\n", mode);
    }

    srand(time(0));
    init_semaphores(waiter_num);

    /* initialize forks and states */
    for (i = 0; i < NUM_PHILS; i++) {
        pthread_mutex_init(&forks[i], NULL);
        state[i] = THINKING;
    }

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