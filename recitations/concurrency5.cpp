#include <pthread.h>
#include <semaphore.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdio.h>

#include <errno.h>
#include <unistd.h>
#include <time.h>
/********
Tanner Fry									
CS 444

This is a demo of the smokers problem
I use 6 smokers, three agents, and an admin to divide up items
To prevent deadlocking I used the timespec structure with the
sem_timedwait that basically puts a limit on how long a semaphore
will hold onto a thread before backing out

Ref:
http://man7.org/linux/man-pages/man3/sem_wait.3.html
http://www.cs.umd.edu/~hollings/cs412/s96/synch/smokers.html
http://www.edutalks.org/downloads/A%20C%20PROGRAM%20TO%20IMPLEMENT%20THE%20SMOKER%27S%20PROBLEM%20USING%20SEMAPHORES.pdf
https://github.com/EvanPurkhiser/CS-SmokersProblem/blob/master/smoke.c
https://www.cs.mtu.edu/~shene/NSF-3/e-Book/SEMA/TM-example-smoker.html

And the Little Book of Semaphores 
*********/				



// Array of the three smokers and the corresponding char array
sem_t smoker_semaphors[3];
char* smoker_needs[3] = { "matches & tobacco", "matches & paper", "tobacco & paper" };
// Type admin for each object/item
sem_t admin_semaphores[3];
// The agent that adds items to the table for the smokers
sem_t agent_lock;
//List of potential objects that are on the table
bool items_available[3] = { false, false, false };

//Setting up time struct to let smoker locks free if enough time has passed
struct timespec ts;

// Handles the waiting and usage of each object
void* smoker(void* arg)
{
	int smoker_id = *(int*) arg;
	int object_num   = smoker_id % 3;
	//Going to smoke four times
	while(1)
	//for (int i = 0; i < 3; ++i)
	{
		//Outputting info
		//Waiting for correct items to be available
		printf("Smoker %d: Waiting for %s...\n", smoker_id, smoker_needs[object_num]);
		sem_timedwait(&smoker_semaphors[object_num], &ts);
		
		//Make and smoke deez joints?
		printf("Smoker %d: Now making the a cigarette... \n", smoker_id);
		usleep(rand() % 500000);
		sem_post(&agent_lock);
		printf("Smoker %d: Now smoking...*puff puff* \n", smoker_id);
		usleep(1000000);
	}

	return NULL;
}

// The agent puts the items on the table
void* agent(void* arg)
{
	int agent_id = *(int*) arg;
	while(1)
	//for (int i = 0; i < 10; ++i)
	{
		usleep(rand() % 200000);

		// Wait for a lock
		// Release both objects
		sem_wait(&agent_lock);
		sem_post(&admin_semaphores[agent_id]);
		sem_post(&admin_semaphores[(agent_id + 1) % 3]);

		// Report the items we placed
		// Fun color string!
		printf("### \033[0;33mAgent %d giving out %s\033[0;0m\n",	agent_id, smoker_needs[(agent_id + 2) % 3]);
	}
}

// This semaphore for the admin to control the objects on the table
sem_t admin_lock;

// The admin handles which smoker gets the items and gets to smoke
void* admin(void* arg)
{
	int admin_id = *(int*) arg;
	// Doing 15 cycles
	// Might change to while loop
	while(1)
	{
		// Waiting until needed
		sem_wait(&admin_semaphores[admin_id]);
		sem_wait(&admin_lock);

		// Are the items on the table that we need?
		if (items_available[(admin_id + 1) % 3]){
			items_available[(admin_id + 1) % 3] = false;
			sem_post(&smoker_semaphors[(admin_id + 2) % 3]);
		}
		
		else if (items_available[(admin_id + 2) % 3]){
			items_available[(admin_id + 2) % 3] = false;
			sem_post(&smoker_semaphors[(admin_id + 1) % 3]);
		}
		
		else{
			items_available[admin_id] = true;
		}

		sem_post(&admin_lock);
	}
}


/**
 * Main function to start and run the program
 * Handles thread management and initallizes the locks
 * Seeds random
 * Using EAGAIN: https://www.ibm.com/support/knowledgecenter/ssw_i5_54/apis/unix14.htm
 * "Operation would have caused the process to be suspended."
 */
int main(int argc, char* arvg[])
{
	srand(time(NULL));
	int smoker_ids[6];
	int admin_ids[6];
	int agent_ids[6];

	ts.tv_sec = 10;
	
	
		// Initialize the smoker and admin semaphores
		for (int i = 0; i < 3; ++i)
		{
			sem_init(&smoker_semaphors[i], 0, 0);
			sem_init(&admin_semaphores[i], 0, 0);
		}

		// Initializing agent semaphore
		sem_init(&agent_lock, 0, 1);
		// Initalizing the admin lock semaphore
		sem_init(&admin_lock, 0, 1);
		
		// Create smoker threads
		pthread_t smoker_threads[6];
		// Pass ids to smoker threads
	
		for (int i = 0; i < 6; ++i)
		{
			smoker_ids[i] = i;

			if (pthread_create(&smoker_threads[i], NULL, smoker, &smoker_ids[i]) == EAGAIN)
			{
				perror("Insufficient resources to create thread");
				return -1;
			}
		}

		// Same deal for admin threads
		// and the agent threads
		pthread_t agent_threads[6];
		pthread_t admin_threads[6];
		for (int i = 0; i < 3; ++i)
		{
			admin_ids[i] = i;

			if (pthread_create(&admin_threads[i], NULL, admin, &admin_ids[i]) == EAGAIN)
			{
				perror("Insufficient resources to create thread");
				return -1;
			}
			
			agent_ids[i] =i;

			if (pthread_create(&agent_threads[i], NULL, agent, &agent_ids[i]) == EAGAIN)
			{
				perror("Insufficient resources to create thread");
				return -1;
			}
		}
		
		// Make sure all the smokers are done smoking
		for (int i = 0; i < 6; ++i)
		{
			pthread_join(smoker_threads[i], NULL);
		}
	
	return 0;
}