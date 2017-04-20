#include <pthread.h>
#include <semaphore.h>
#include <stdio.h>
#include <stdlib.h>
#include "mt19937ar.h"

typedef struct {
   int item;
   int wait;
} buff_item;

buff_item buffer[32];

int check_buffer();

void consumer(void*);

void producer(void*);

int main() {


   return 0;
}
           
