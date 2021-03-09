#include <pthread.h> 
#include <stdio.h> 
#include <stdlib.h> 
#include <string.h> 
#include <unistd.h> 
#include "../rpthread.h"

int counter; 
pthread_mutex_t lock;
void* trythis(void* arg) 
{

	pthread_mutex_lock(&lock);
	unsigned long i = 0;
	counter += 1; 
	printf("\n\n Job %d has started\n\n", counter); 

	for(i = 0; i < (0x00FFFFFF);i++);
	printf("\n\n Job %d has finished\n\n", counter);
	pthread_mutex_unlock(&lock);
	pthread_exit(0); 
} 

int main(void) 
{ 
	int i = 0; 
 	pthread_t tid,tid2; 
	pthread_mutex_init(&lock,NULL);

	pthread_create(&(tid), NULL, &trythis, NULL);
	pthread_create(&(tid2),NULL,&trythis,NULL);

	pthread_join(tid2,NULL);
	pthread_join(tid,NULL);

	printf("when is thsi happening???\n");

	return 0; 
} 

