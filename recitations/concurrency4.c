#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#define seats 4
//Tanner Fry group 11-01
//References:
// http://www.dreamincode.net/forums/topic/47521-barber-shop-problem/
// http://greenteapress.com/semaphores/LittleBookOfSemaphores.pdf
// http://sce2.umkc.edu/csee/cotterr/cs431_sp13/barber.html
// http://www.cs.csi.cuny.edu/~yumei/csc718/homework2/homework2solution.pdf


//Function prototypes
void *customer_thread_func();
void *barber_shop();
void *waitingRoom();
void checkQueue();

//Threads and mutexes
pthread_mutex_t queue_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t wait_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t sleep_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t barberSleep_cond = PTHREAD_COND_INITIALIZER;
pthread_cond_t barber_active_cond = PTHREAD_COND_INITIALIZER;

//Globals
int return_time=5,current=0, sleeping=0, iseed;

int main(int argc, char *argv[])
{
	iseed=time(NULL);
	srand(iseed);
	//declare barber thread
	pthread_t barber,customer_thread,timer_thread;
	//declare attributes
	pthread_attr_t barber_attr, timer_attr;
	pthread_attr_t customer_attr;

	//define barber and customer attributes
	pthread_attr_init(&timer_attr);
	pthread_attr_init(&barber_attr);
	pthread_attr_init(&customer_attr);

	//create customer
	pthread_create(&customer_thread,&customer_attr,customer_thread_func,NULL);

	//create barber
	pthread_create(&barber,&barber_attr,barber_shop,NULL);

	pthread_join(barber,NULL);
	pthread_join(customer_thread,NULL);
	
	printf("\n");

	return 0;
}

void *customer_thread_func()
{
	int i=0;
	printf("*Customer Maker Created*\n");
	printf("*This Barber Shop has %i chairs* \n",seats);
	fflush(stdout);
	//Increment number of seats 
	pthread_t customer[seats+1];
	pthread_attr_t customerAttr[seats+1];
	while(i<(seats+1))
	{
		i++;
		pthread_attr_init(&customerAttr[i]);
		while(rand()%2!=1)
		{
				sleep(3);
		}
		pthread_create(&customer[i],&customerAttr[i],waitingRoom,NULL);
	}
	pthread_exit(0);
}

void *waitingRoom()
{
	//taking the seat mang
	pthread_mutex_lock(&queue_mutex);
	checkQueue();
	//sleep for set amount of time
	sleep(return_time);
	waitingRoom();
}

void *barber_shop()
{
	int loop=0;
	printf("Barber thread started, store open!\n");
	fflush(stdout);
	while(loop==0)
	{
		if(current==0)
		{
			printf("No customers, barber is sleeping\n");
			fflush(stdout);
			//Locking barber with sleep
			pthread_mutex_lock(&sleep_mutex);
			sleeping=1;
			pthread_cond_wait(&barberSleep_cond,&sleep_mutex);
			sleeping=0;
			//Unlocking barber sleep
			pthread_mutex_unlock(&sleep_mutex);
			printf("Barber wakes up.\n");
			fflush(stdout);
		}
		else
		{
			printf("Barber begins cutting hair.\n");
			fflush(stdout);
			sleep((rand()%15)/5);
			//Decrement count
			current--;
			printf("Completed a hair cut, customer is leaving. \n");
			pthread_cond_signal(&barber_active_cond);
		}
	}
	pthread_exit(0);
}

void checkQueue()
{
	//increment count
	current++;
	printf("A new Customer has arrived in the shop. There are %d Customers in store.\n",current);
	fflush(stdout);
	printf("Customer checking chairs.\n");
	fflush(stdout);
	if(current<seats)
	{
		if(sleeping==1)
		{
			printf("Barber was sleeping, customer wakes him up.\n");
			fflush(stdout);
			pthread_cond_signal(&barberSleep_cond);
		}
		printf("Customer grabs an empty chairs.\n");
		fflush(stdout);
		//unlocking and grabing chair
		pthread_mutex_unlock(&queue_mutex);
		pthread_mutex_lock(&wait_mutex);
		pthread_cond_wait(&barber_active_cond,&wait_mutex);
		pthread_mutex_unlock(&wait_mutex);
		return;
	}
	if(current>=seats)
	{
		printf("All the chairs are taken, customer leaving the shop...\n");
		fflush(stdout);
		//decrement count
		current--;
		//unlocking the chairs
		pthread_mutex_unlock(&queue_mutex);
		return;
	}
}
