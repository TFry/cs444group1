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
   sem_t no_searcher, no_inserter, inserter;
} list_t;

typedef struct {
   int thread_num;
} param_t;

int random_gen()
{
   return rand() % 10 + 1;
}

list_t list;

void print_list(list_t *lst) {
      struct node *cur;

      cur = lst->head;

      while(cur != NULL) {
            printf("[%d] ", cur->data);
            cur = cur->next;
      }
      printf("\n");
}


void remov(list_t *lst, int i)
{
   int val = random_gen();
   int j, found = 0;
   struct node *prev, *cur, *tmp;
   
   prev = cur = lst->head;
   
   sem_wait(&(lst->no_searcher));
   sem_wait(&(lst->no_inserter));
   if (lst->head != NULL && lst->head->data == val)
   {
      found = 1;
      tmp = lst->head->next;
      free(lst->head);
      lst->head = tmp;
   } else {
      while (cur != NULL)
      {
      //    printf("%d %d\n", cur->data, val);
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
   }

   if (found == 1)
   {
      printf("[Thread %d] Deleting value: %d\n", i, val);
      print_list(lst);
   }

   sem_post(&(lst->no_searcher));
   sem_post(&(lst->no_inserter));

}

void find(list_t *lst, int i)
{
   struct node *tmp = lst->head;
   int val = random_gen();
   sem_wait(&(lst->no_searcher));
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
   sem_post(&(lst->no_searcher));
}

void create(list_t *lst, int i)
{
      struct node *new_node = malloc(sizeof(struct node));;
      int val = random_gen();
      struct node *cur;
      
      new_node->next = NULL;
      new_node->data = val;
      
      sem_wait(&(lst->no_inserter));
      sem_wait(&(lst->inserter));

      if (lst->head == NULL)
      { 
         lst->head = new_node;
      } else {
         cur = lst->head;
         while(cur->next != NULL) {
            cur = cur->next;
         }
         cur->next = new_node;
      }


      printf("[Thread %d] Inserting value: %d\n", i, val);
      print_list(lst);
      sem_post(&(lst->inserter));
      sem_post(&(lst->no_inserter));
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
   srand(time(NULL));
   lst->head = NULL;
   lst->tail = NULL;
   
   sem_init(&(lst->no_searcher), 0, 1);
   sem_init(&(lst->no_inserter), 0, 1);
   sem_init(&(lst->inserter), 0, 1);
}

int main()
{
   int i;
   pthread_t searchers[2], deleters[2], inserters[2];
   param_t params;

   initialize(&list);

   for (i = 0; i < 2; i++)
   {
      params.thread_num = i + 5;
      pthread_create(&inserters[i], NULL, inserter, &params);
   }

   for (i = 0; i < 2; i++)
   {
      params.thread_num = i + 1;
      pthread_create(&searchers[i], NULL, searcher, &params);
   }

   for (i = 0; i < 2; i++)
   {
      params.thread_num = i + 3;
      pthread_create(&deleters[i], NULL, deleter, &params);
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
