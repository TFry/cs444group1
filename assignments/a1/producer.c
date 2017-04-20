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
	buff_item buffer[32];
	pthread_mutex_t lock;
	int producer_buff_location;
	int consumer_buff_location;
} buffer_list;


buffer_list buffer;
pthread_cond_t consumer_condition, producer_condition;
int check_buffer();

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
   void* consume_func = consumer;
   void* produce_func = producer;
   pthread_cond_init(&consumer_condition, NULL);
   pthread_cond_init(&producer_condition, NULL);
   
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
   for (i = 0; i < 32; i++)
   {
      buffer.buffer[i].item = 0;
      buffer.buffer[i].wait = 0;
   }
   
   pthread_mutex_init(&(buffer.lock), NULL);
   
   for (i = 0; i < num_threads; i++)
   {
      pthread_create(&producers[i], NULL, produce_func, NULL);
      pthread_create(&consumers[i], NULL, consume_func, NULL);
      pthread_join(producers[i], NULL);
      pthread_join(consumers[i], NULL);
   }
   
   pthread_mutex_destroy(&(buffer.lock));
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

unsigned long random_gen(int top_val, int low_val) 
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
	
	if (0)
	{
      rdrand32_step(&x);
      return x % top_val + low_val;
	}
	
	else
	{
	   init_genrand(seed);
 	   return abs(genrand_int32()) % top_val + low_val;
	}

}

buff_item produce_item()
{
   buff_item new_item;
   new_item.wait = random_gen(2, 9);
   new_item.item = random_gen(1, 50);
   
   return new_item;
}
/*
void consumer(void * data) {
   // Lock
   pthread_mutex_lock(&buffer.lock);

   while(buffer.producer_buff_location == 0) {
      sleep(1);
   }
   buff_item item = buffer.buffer[buffer.consumer_buff_location];
   buffer.consumer_buff_location++;
   if(buffer.consumer_buff_location >= 32) {
       buffer.consumer_buff_location = 0;
   }
   int value = item.item;
   int wait  = item.wait;
   printf("%d \n", value);
   sleep(wait);
   pthread_mutex_unlock(&buffer.lock);
}
*/

int check_buffer()
{
   int i = -1;
   
   while(buffer.buffer[i + 1].item != 0)
   {
      i++;
   }
   
   return i;
}
