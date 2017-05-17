#include <pthread.h>
#include <semaphore.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <stdint.h>
#include <unistd.h>

struct node {
        int data;
        struct node *next;
};

typedef struct {
   struct node *head, *tail;
   sem_t sem;
   pthread_mutex_t ins, del;
} list_t;

typedef struct {
   int thread_num;
} param_t;

int random_gen()
{
   return rand() % 10 + 1;
}

list_t list;


void remov(list_t *lst, int i)
{
   int val = random_gen();
   int j, found = 0;
   struct node *prev, *cur, *tmp;
   
   prev = cur = lst->head;

   for (j = 0; j < 4; j++)
   {
      sem_wait(&(lst->sem));
   }


   pthread_mutex_lock(&(lst->ins));
   pthread_mutex_lock(&(lst->del));

   if (lst->head != NULL && lst->head->data == val)
   {
      found = 1;
      tmp = lst->head->next;
      free(lst->head);
      lst->head = tmp;
   }

   else
   {
      while (cur != NULL)
      {
         if (cur->data == val)
         {
            found = 1;
            prev->next = cur->next;
            if (lst->tail == cur)
            {
               lst->tail = prev;
            }
            free(cur);
            break;
         }
         prev = cur;
         cur = cur->next;
      }
      
      if (found == 1)
      {
         printf("[Thread %d] Deleting value: %d\n", i, val);
      }
   }

   pthread_mutex_unlock(&(lst->ins));
   pthread_mutex_unlock(&(lst->del));
   for (j = 0; j < 4; j++)
   {
      sem_post(&(lst->sem));
   } 

}

void find(list_t *lst, int i)
{
   struct node *tmp = lst->head;
   int val = random_gen();
   sem_wait(&(lst->sem));
   printf("[Thread %d] Searching for value: %d\n", i, val);
   while (tmp != NULL)
   {
      if (tmp->data == val)
      {
         printf("[Thread %d] Found value: %d\n", i, val);
         break;
      }
         
      tmp = tmp->next;
   }
   sem_post(&(lst->sem));
}

void create(list_t *lst, int i)
{
      struct node *new_node = malloc(sizeof(struct node));;
      int val = random_gen();
      
      new_node->next = NULL;
      new_node->data = val;
      
      sem_wait(&(lst->sem));
      pthread_mutex_lock(&(lst->ins));


      if (lst->head == lst->tail)
      { 
         lst->head = new_node;
      }

      if (lst->tail != NULL)
      {
         lst->tail->next = new_node;
         lst->tail = new_node;
      }

      if (lst->tail == NULL)
      {
         lst->tail = new_node;
      }

      printf("[Thread %d] Inserting value: %d\n", i, val);

      pthread_mutex_unlock(&(lst->ins));
      sem_post(&(lst->sem));
}

void *searcher(void *params)
{
   param_t *args = params;
   int i = args->thread_num;

   while(1)
   {
      find(&list, i);
      sleep(random_gen());
   }
}


void *deleter(void *params)
{
   param_t *args = params;
   int i = args->thread_num;

   while(1)
   {    
      remov(&list, i);
      sleep(random_gen());     
   }
}



void *inserter(void *params)
{
   param_t *args = params;
   int i = args->thread_num;

   while(1)
   {
      create(&list, i);
      sleep(random_gen());
   }
}





void initialize(list_t *lst)
{
   pthread_mutex_t x = PTHREAD_MUTEX_INITIALIZER;
   srand(time(NULL));
   lst->head = NULL;
   lst->tail = NULL;
   
   pthread_mutex_init(&(lst->ins), NULL);

   pthread_mutex_init(&(lst->del), NULL);
   
   sem_init(&(lst->sem), 0, 4);
}

int main()
{
   int i;
   pthread_t searchers[2], deleters[2], inserters[2];
   param_t params;

   initialize(&list);

   for (i = 0; i < 2; i++)
   {
      params.thread_num = i + 1;
      pthread_create(&searchers[i], NULL, searcher, &params);
   }

   for (i = 0; i < 2; i++)
   {
      params.thread_num = i + 1;
      pthread_create(&deleters[i], NULL, deleter, &params);
   }

   for (i = 0; i < 2; i++)
   {
      params.thread_num = i + 1;
      pthread_create(&inserters[i], NULL, inserter, &params);
   }

   for (i = 0; i < 2; i++)
   {
      params.thread_num = i + 1;
      pthread_join(searchers[i], NULL);
   }

   for (i = 0; i < 2; i++)
   {
      params.thread_num = i + 1;
      pthread_join(deleters[i], NULL);
   }

   for (i = 0; i < 2; i++)
   {
      params.thread_num = i + 1;
      pthread_join(inserters[i], NULL);
   }

   return 0;

}
