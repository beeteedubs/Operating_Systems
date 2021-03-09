#include "../basics.h"
#include <pthread.h>

#define MAX_CORES 12
#define COUNT_TO 1000

long long total = 0;
pthread_mutex_t lock;

void* start_counting(void* arg){
	while(1){
		//pthread_mutex_lock(&lock);
		if (total>=COUNT_TO){
			return NULL;
		}
		++total;
		printf("total = %lld\n",total);
		/*
		total = total + 1;
		//pthread_mutex_unlock(&lock);
		printf("total: %d\t tid: %ld\n",total,*(long int*)tid%9);
		i++;*/
	}
	pthread_exit(0);
}


int main (int argc, char** argv){
	int i = 0;
	
	pthread_t *thread_group = malloc(sizeof(pthread_t)*MAX_CORES);

	pthread_mutex_init(&lock,NULL);
	
	for(i = 0; i < MAX_CORES; i++){
		pthread_create(&thread_group[i], NULL,start_counting,NULL);
	}
	
	for(i = 0; i < MAX_CORES; i++){
		pthread_join(thread_group[i],NULL);
	}/*
	int a = 5;
	int *ptr = &a;
	printf("curr contents: %d\n",*ptr);
	printf("calling it\n");
	__sync_lock_test_and_set(ptr,1);
	while(__sync_lock_test_and_set(ptr,1)==1){
		sleep(1);
		printf("prev contents: %d\n",*ptr);
	}*/
	printf("Final total: %lld\n",total);
	return 0;
}
