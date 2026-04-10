/*
This code was taken from supplied lab material as a basic framework to develop our project
*/
#include <stdio.h>
#include <unistd.h>
#include <pthread.h>
#include <semaphore.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>

#define NUM_PHILS 5     
#define THINKING 0
#define HUNGRY 1
#define EATING 2

#define GREEN "\033[92m"   //EATING
#define RED "\033[91m"     //HUNGRY
#define BLUE "\033[94m"    //THINKING
#define YELLOW "\033[93m"  //MEALS
#define RESET "\033[0m"

pthread_mutex_t forks[NUM_PHILS];
int state[NUM_PHILS];
char *phil_names[NUM_PHILS] = {"Aristotle", "Plato", "Socrates", "Descartes", "Kant"};
int meals[NUM_PHILS] = {0};

void *think_and_eat(void *);
int left_fork(int i);
int right_fork(int i);

sem_t mutex;
sem_t waiter;

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
      }
    }

    if (mode != 0 && mode != 1 && mode != 2){
      printf("Invalid mode selected, exiting program\n");
      exit(1);
    }

    if (steps != 1){
      printf("Running mode=%d for %d steps\n", mode, steps);
    }
    else {
      printf("Running mode=%d\n", mode);
    } 
    
    srand(time(0)); //Seeding random times for different run results
    sem_init(&mutex, 0, 1); 
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
    int count = 0;
    
    while (steps == -1 || count < steps){
      
    //Thinking
    sem_wait(&mutex); 
    state[i] = THINKING;
    printf("%sP%ld %s is THINKING%s\n", BLUE, i, phil_names[i], RESET);
    sem_post(&mutex); 

    usleep(500000); // think for 0.5 seconds
    
    //Hungry
    sem_wait(&mutex);
    state[i] = HUNGRY;
    printf("%sP%ld %s is HUNGRY%s\n", RED, i, phil_names[i], RESET);
    sem_post(&mutex);


    // MODE 2 - WAITER SOLUTION — only wraps fork pickup
    if (mode == 2){
      sem_wait(&waiter);  /* take a seat — blocks if 4 already seated */
    }

    // MODE 1 - ASYMMETRIC SOLUTION
    if (mode == 1 && i == NUM_PHILS - 1) {
        
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
	sem_wait(&mutex);
	state[i] = EATING;
	meals[i]++;
	printf("%sP%ld %s is EATING%s\n", GREEN, i, phil_names[i], RESET);
	printf("%sP%ld %s have eaten %d meal(s)%s\n", YELLOW, i, phil_names[i], meals[i], RESET);
	sem_post(&mutex);

	
	
	usleep(500000); // eat for 2.5 seconds

	//Put forks down then release waiter seat
	pthread_mutex_unlock(&forks[right_fork(i)]);
	pthread_mutex_unlock(&forks[left_fork(i)]);

	if (mode == 2){
	  sem_post(&waiter);    /* leave the table — allow next philosopher in */
	}
	  
	count++;
}

    return 0;
}

int left_fork(int i){
  return i;
}

int right_fork(int i){
  return (i + 1) % NUM_PHILS;
}
