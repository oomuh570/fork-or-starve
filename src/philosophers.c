/*
This code was taken from supplied lab material as a basic framework to develop our project
*/

#include <stdio.h>
#include <unistd.h>
#include <pthread.h>
#include <semaphore.h>
#include <stdlib.h>
#include <time.h>

#define NUM_PHILS 5     
#define THINKING 0
#define HUNGRY 1

int state[NUM_PHILS];
char *phil_names[NUM_PHILS] = {"Aristotle", "Plato", "Scorates", "Descartes", "Kant"};

void *think_and_eat(void *);

sem_t mutex;

int main()
{
    pthread_t philosopher[NUM_PHILS];
    long i;
    srand(time(0)); //Seeding random times for different run results
    sem_init(&mutex, 0, 1);

    //Creating concurrent threads
    for (i = 0; i < NUM_PHILS; i++)
      pthread_create(philosopher + i, 0, think_and_eat, (void *)(long)i);

    //Joining threads
    for (i = 0; i < NUM_PHILS; i++)
      pthread_join(philosopher[i], NULL);
    
    return 0;
}


void *think_and_eat(void *arg)     /* executed concurrently by all philosophers */
{
    long i = (long)arg;

    while (1){
      
	//Thinking
	sem_wait(&mutex);
	state[i] = THINKING;
	printf("P%ld %s is THINKING\n", i, phil_names[i]);
	sem_post(&mutex);

	//Sleeps for between 2-3 seconds
	usleep((rand() % 1000 + 2000) * 1000);

	//Hungry
	sem_wait(&mutex);
	state[i] = HUNGRY;
	printf("P%ld %s is HUNGRY\n", i, phil_names[i]);
	sem_post(&mutex);

	//Sleeps for between 2-3 seconds
	usleep((rand() % 1000 + 2000) * 1000);
	
    }

    return 0;
}

