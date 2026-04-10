#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <string.h>
#include <time.h>

#include "philosopher.h"
#include "semaphore.h"

pthread_mutex_t forks[NUM_PHILS];
int state[NUM_PHILS];
char *phil_names[NUM_PHILS] = {"Aristotle", "Plato", "Socrates", "Descartes", "Kant"};
int meals[NUM_PHILS] = {0};

int mode = 0;
int steps = -1;

int main(int argc, char *argv[])
{
  
    pthread_t philosopher[NUM_PHILS];
    long i;

    for (int i = 1; i < argc; i++) {
      if (strncmp(argv[i], "--mode=", 7) == 0) {
	mode = atoi(argv[i] + 7);     //quora.com/How-do-I-extract-an-integer-from-the-string-in-C
      }
      else if (strncmp(argv[i], "--steps=", 8) == 0) {
	steps = atoi(argv[i] + 8);
      }
      else {
	printf("Unknown argument \"%s\" detected\n", argv[i]);
	exit(1);
      }
    }

    if (mode != 0 && mode != 1 && mode != 2){
      printf("Invalid mode selected, exiting program\n");
      exit(1);
    }

    if (steps != -1){
      printf("Running mode=%d for %d steps\n", mode, steps);
    }
    else {
      printf("Running mode=%d\n", mode);
    } 
    
    srand(time(0)); //Seeding random times for different run results
    init_semaphores();

    //Creating mutexs for forks
    for (i = 0; i < NUM_PHILS; i++){
      pthread_mutex_init(&forks[i], NULL);
    }
    
    //Creating concurrent threads
    for (i = 0; i < NUM_PHILS; i++)
      pthread_create(philosopher + i, 0, think_and_eat, (void *)(long)i);

    //Joining threads
    for (i = 0; i < NUM_PHILS; i++)
      pthread_join(philosopher[i], NULL);
    
    return 0;
}
