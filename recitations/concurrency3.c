#include <pthread.h>
#include <semaphore.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

pthread_mutex_t s_lock, d_lock, i_lock;

typedef struct
{
   int data;
   struct list *next;
} list;

list *head;

int rand()
{
   return rand() % 10 + 1;
}

void *search(int i)
{
   struct list *tmp;
   
   while (1)
   {
      if (!pthread_mutex_trylock(&s_lock))
      {
         tmp = head;
         while (tmp != NULL)
         {
         
         }
      }

   }


}

void *delete(int i)
{



}

void *insert(int i)
{


}



int main()
{
   int i;
   pthread_t searchers[2], deleters[2], inserters[2];

   srand(time(NULL));

   for (i = 0; i < 2; i++)
   {
      pthread_create(&searchers[i], NULL, search, (void *)i);
   }

   for (i = 0; i < 2; i++)
   {
      pthread_create(&deleters[i], NULL, delete, (void *)i);
   }

   for (i = 0; i < 2; i++)
   {
      pthread_create(&inserters[i], NULL, insert, (void *)i);
   }

   for (i = 0; i < 2; i++)
   {
      pthread_join(searchers[i], NULL);
   }

   for (i = 0; i < 2; i++)
   {
      pthread_join(deleters[i], NULL);
   }

   for (i = 0; i < 2; i++)
   {
      pthread_join(inserters[i], NULL);
   }

   return 0;
}

