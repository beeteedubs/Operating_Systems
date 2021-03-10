#include <pthread.h> 
#include <stdio.h> 
#include <stdlib.h> 
#include <string.h> 
#include <unistd.h> 
#include "../rpthread.h"
#define DEFAULT_THREAD_NUM 3
int thread_num;
int counter; 

pthread_mutex_t lock;
void* trythis(void* arg) 
{

	pthread_mutex_lock(&lock);
	unsigned long i = 0;
	counter += 1; 
	printf("Job %d has started\n", counter); 

	for(i = 0; i < (0x00FFFFFF);i++);//fake work (more than 5ms)
	printf("Job %d has finished\n", counter);
	pthread_mutex_unlock(&lock);
	pthread_exit(0); 
} 

int main(int argc, char**argv) 
{
pthread_t* tids = (pthread_t*)malloc(thread_num*sizeof(pthread_t));; 
	if (argc == 1) {
		thread_num = DEFAULT_THREAD_NUM;
	} else {
		if (argv[1] < 1) {
			printf("enter a valid thread number\n");
			return 0;
		} else
			thread_num = atoi(argv[1]);
	}	
	int i = 0; 
	pthread_mutex_init(&lock,NULL);
	for(i=0;i<thread_num;i++){
		pthread_create(&tids[i], NULL, &trythis, NULL);
	}
	for(i=0;i<thread_num;i++){
		pthread_join(tids[i], NULL);
	}
	free(tids);
//	pthread_create(&(tid2),NULL,&trythis,NULL);

//	pthread_join(tid2,NULL);
//	pthread_join(tid,NULL);

	printf("when is thsi happening???\n");

	return 0; 
} 

