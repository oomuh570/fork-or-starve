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
#define EATING 2

pthread_mutex_t forks[NUM_PHILS];
int state[NUM_PHILS];
char *phil_names[NUM_PHILS] = {"Aristotle", "Plato", "Socrates", "Descartes", "Kant"};

void *think_and_eat(void *);
int left_fork(int i);
int right_fork(int i);

//sem_t mutex; //Mark for removal
sem_t waiter;

int main()
{
    pthread_t philosopher[NUM_PHILS];
    long i;
    srand(time(0)); //Seeding random times for different run results
    //sem_init(&mutex, 0, 1); //Mark for removal
	sem_init(&waiter, 0, 4);

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


void *think_and_eat(void *arg)     /* executed concurrently by all philosophers */
{
    long i = (long)arg;

    while (1){
      
	//Thinking
	//sem_wait(&mutex); //Mark for removal
	sem_wait(&waiter);
	state[i] = THINKING;
	printf("P%ld %s is THINKING\n", i, phil_names[i]);
	//sem_post(&mutex); //Mark for removal
	sem_post(&waiter);

	usleep(500000); // think for 0.5 seconds
	
	//Hungry
	//sem_wait(&mutex); //Mark for removal
	sem_wait(&waiter);
	state[i] = HUNGRY;
	printf("P%ld %s is HUNGRY\n", i, phil_names[i]);
	//sem_post(&mutex); //Mark for removal
	sem_post(&waiter);


 // ASYMMETRIC FORK PICKUP
    if (i == NUM_PHILS - 1) {
        
        // P4 picks up RIGHT fork first
        pthread_mutex_lock(&forks[right_fork(i)]);
        printf("P%ld %s picked up RIGHT fork %d\n", i, phil_names[i], right_fork(i));
        
        usleep(500000);
        
        pthread_mutex_lock(&forks[left_fork(i)]);
        printf("P%ld %s picked up LEFT fork %d\n", i, phil_names[i], left_fork(i));
    } 
    else {
        
        // Everyone else picks up LEFT fork first
        pthread_mutex_lock(&forks[left_fork(i)]);
        printf("P%ld %s picked up LEFT fork %d\n", i, phil_names[i], left_fork(i));
        
        usleep(500000); 
        
        pthread_mutex_lock(&forks[right_fork(i)]);
        printf("P%ld %s picked up RIGHT fork %d\n", i, phil_names[i], right_fork(i));
    }

	//EATING
	//sem_wait(&mutex); //Mark for removal
	sem_wait(&waiter);
	state[i] = EATING;
	printf("P%ld %s is EATING\n", i, phil_names[i]);
	//sem_post(&mutex); //Mark for removal
	sem_post(&waiter);
	
	
	usleep(500000); // eat for 0.5 seconds

	//Put forks down
	pthread_mutex_unlock(&forks[right_fork(i)]);
	pthread_mutex_unlock(&forks[left_fork(i)]);
    }

    return 0;
}

int left_fork(int i){
  return i;
}

int right_fork(int i){
  return (i + 1) % NUM_PHILS;
}