// Reference heavily used for this code:
// http://pseudomuto.com/development/2014/03/01/dining-philosophers-in-c/
// Make sure to compile using the "-pthreads" flag!

#include <stdio.h>
#include <pthread.h>
#include <semaphore.h>
#include <stdlib.h>

typedef struct {
   int pos;
   int count;
   sem_t *lock;
   sem_t *forks;
} params;

void run(pthread_t*, sem_t*, sem_t*, int);

void init(sem_t*, sem_t*, int);

void *phil(void*);

int main()
{
   int num_phil = 4;
   sem_t lock;
   sem_t forks[num_phil];
   pthread_t phils[num_phil];

   init(&lock, forks, num_phil);

   run(phils, forks, &lock, num_phil);

   pthread_exit(NULL);

   return 0;

}


void init(sem_t *lock, sem_t *forks, int num_forks)
{
   int i;
   
   for (i = 0; i < num_forks; i++)
   {
      sem_init(&forks[i], 0, 1);
   }
   
   sem_init(lock, 0, num_forks - 1);

}


void run(pthread_t *threads, sem_t *forks, sem_t *lock, int num_phil)
{
   int i;

   for(i = 0; i < num_phil; i++)
   {
      params *p = malloc(sizeof(params));
      p->pos = i;
      p->count = num_phil;
      p->lock = lock;
      p->forks = forks;

      pthread_create(&threads[i], NULL, phil, (void*)p);
   }
}


void *phil(void *param)
{
   int i;
   params self = *(params*)param;

   for(i = 0; i < 3; i++)
   {
      printf("Philosopher %d is thinking.\n", self.pos + 1);

      sem_wait(self.lock);
      sem_wait(&self.forks[self.pos]);
      sem_wait(&self.forks[(self.pos + 1) % self.count]);
      
      printf("Philosopher %d is eating.\n", self.pos + 1);

      sem_post(&self.forks[self.pos]);
      sem_post(&self.forks[(self.pos + 1) % self.count]);
      sem_post(self.lock);
   }

   printf("Philosopher %d is thinking.\n", self.pos + 1);
   pthread_exit(NULL);
}

