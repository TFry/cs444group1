#include <pthread.h>
#include <semaphore.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>
#include "mt19937ar.c"

typedef struct 
{
   int item;
   int wait;
} buff_item;

typedef struct 
{
	buff_item items[32];
	pthread_mutex_t lock;
	int last_item;
} buffer_list;


buffer_list buffer;

pthread_cond_t prod_cond;

void consumer(void*);

void producer(void*);

buff_item produce_item();

unsigned long random_gen();

int main(int argc, char **argv) 
{
	
   int num_threads;
   int i;
   pthread_t *producers;
   pthread_t *consumers;
  
   pthread_cond_init(&prod_cond, NULL);
   
   if (argc <= 1)
   {
      num_threads = 2;
   }
   
   else
   {
      num_threads = atoi(argv[1]);
   }
   
   producers = malloc(sizeof(pthread_t) * num_threads);
   
   consumers = malloc(sizeof(pthread_t) * num_threads);
   
   // initialize buffer

   buffer.last_item = -1;
   for (i = 0; i < 32; i++)
   {
      buffer.items[i].item = 0;
      buffer.items[i].wait = 0;
   }
   
   // initialize mutex   
   pthread_mutex_init(&(buffer.lock), NULL);
   
   // initialize threads
   for (i = 0; i < num_threads; i++)
   {
      // pass in functions to threads
      pthread_create(&producers[i], NULL, (void*)consumer, NULL);
      pthread_create(&consumers[i], NULL, (void*)producer, NULL);
      pthread_join(consumers[i], NULL);
      pthread_join(producers[i], NULL);
   }
   
   // clean up
   pthread_mutex_destroy(&(buffer.lock));

   pthread_cond_destroy(&prod_cond);

   free(producers);
   free(consumers);
   
   return 0;
   
}


int rdrand32_step (uint32_t *rand) {
    unsigned char ok;
    __asm__ __volatile__("rdrand %0; setc %1"
                    : "=r" (*rand), "=qm" (ok));
    return (int) ok;
}

// taken from course materials and Intel documentation
unsigned long random_gen() 
{

	unsigned int eax;
	unsigned int ebx;
	unsigned int ecx;
	unsigned int edx;
	int num;
	
	unsigned long seed;

	uint32_t x;

	char vendor[13];
	
	eax = 0x01;

	__asm__ __volatile__(
	                     "cpuid;"
	                     : "=a"(eax), "=b"(ebx), "=c"(ecx), "=d"(edx)
	                     : "a"(eax)
	                     );
	
	if (ecx & 0x40000000)
	{
           rdrand32_step(&x);
           return x;
	}
	
	else
	{
	   init_genrand(seed);
 	   return abs(genrand_int32());
	}

}

// creates new item. arbitrarily chose 1 and 50 as bounds
// since it is not specified in assignment.
buff_item produce_item()
{
   buff_item new_item;
   new_item.wait = random_gen() % 8 + 2;
   new_item.item = random_gen() % 50 + 1;
   
   return new_item;
}

// runs the consumer thread
void consumer(void *data)
{
   int sleep_time;

   while (1)
   {
      if (buffer.last_item > -1)
      {
         pthread_mutex_lock(&(buffer.lock));   
         
         pthread_mutex_unlock(&(buffer.lock));
 
         printf("Consumer sleeping %d seconds.\n", buffer.items[buffer.last_item].wait);
         sleep(buffer.items[buffer.last_item].wait);

         printf("Consumed: item = %d, wait = %d.\n", buffer.items[buffer.last_item].item, buffer.items[buffer.last_item].wait);

         pthread_mutex_lock(&(buffer.lock));
         buffer.items[buffer.last_item].item = 0;
         buffer.items[buffer.last_item].wait = 0;
         buffer.last_item--;

         pthread_cond_signal(&prod_cond);
         pthread_mutex_unlock(&(buffer.lock));
      }
   }
   pthread_exit(NULL);
}

// runs the producer thread
void producer(void *data)
{
   int sleep_time;

   while (1)
   {      
      sleep_time = random_gen() % 5 + 3;

      printf("Producer is sleeping %d seconds.\n", sleep_time);
 
      sleep(sleep_time);

      pthread_mutex_lock(&(buffer.lock));

      // wait until buffer has available space
      while (buffer.last_item == 31)
      {
         pthread_cond_wait(&prod_cond, &(buffer.lock));
      }
      
      // "produce"
      buffer.items[buffer.last_item + 1] = produce_item();
      buffer.last_item++;   

      printf("Produced: item = %d, wait = %d\n", buffer.items[buffer.last_item].item, buffer.items[buffer.last_item].wait);
      
      pthread_mutex_unlock(&(buffer.lock));
       
   }
   // clean up
   pthread_exit(0);
}

