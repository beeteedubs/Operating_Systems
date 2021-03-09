#include <pthread.h> 
#include <stdio.h> 
#include <stdlib.h> 
#include <string.h> 
#include <unistd.h> 
//#include "../rpthread.h" // comment out to test regular pthreads

pthread_t tid1, tid2; 
int counter; 

void* trythis(void* arg) 
{ 
	unsigned long i = 0; 
	counter += 1; 
	printf("\n Job %d has started\n", counter); 

	sleep(2);
	printf("\n Job %d has finished\n", counter); 

	return NULL; 
} 

int main(void) 
{ 
	int i = 0; 
	int error; 

	pthread_create(&tid1, NULL, &trythis, NULL); 
	pthread_create(&tid1, NULL, &trythis, NULL); 

	pthread_join(tid1, NULL); 
	//pthread_join(tid2, NULL); 

	return 0; 
} 

