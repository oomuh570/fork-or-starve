#include "philosopher.h"
#include "forks.h"
#include "semaphore.h"
#include "display.h"
#include "stats.h"
#include <unistd.h>
#include <stdio.h>

void *think_and_eat(void *arg)     /* executed concurrently by all philosophers */
{
    long i = (long)arg;
    int count = 0;
    
    while (steps == -1 || count < steps){
      
    //Thinking
    sem_wait(&mutex); 
    state[i] = THINKING;
    print_state(i, THINKING);
    sem_post(&mutex); 

    usleep(500000); // think for 0.5 seconds
    
    //Hungry
    sem_wait(&mutex);
    state[i] = HUNGRY;
    print_state(i, HUNGRY);
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
	print_state(i, EATING);
	meal_record(i);
	sem_post(&mutex);

	
	
	usleep(500000); // eat for 0.5 seconds

	//Put forks down then release waiter seat
	pthread_mutex_unlock(&forks[right_fork(i)]);
	pthread_mutex_unlock(&forks[left_fork(i)]);

	if (mode == 2){
	  sem_post(&waiter);    /* leave the table — allow next philosopher in */
	}
	
	sem_wait(&mutex);
	starve_check(count);
	sem_post(&mutex);
	  
	count++;
}
    return 0;
}
