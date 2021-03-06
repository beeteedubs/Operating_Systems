#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include "../rpthread.h"

long long sum = 0;

void* myturn(void* args){
	for(int i = 0; i<8; i++){
		sleep(1);
		printf("myturn\n");
	}
	return NULL;
}

void yourturn(){
	for(int i = 0; i<8; i++){
		sleep(1);
		printf("yourturn\n");
	}
}

void* sum_runner(void* args){
	printf("In sum_runner!\n");

	for (long long i= 0; i <=5; i++){
		sum += i;
	}
	printf("Sum == %d\n", sum);
	pthread_exit(0);
}
int main(int argc, char **argv) {
	printf("In main!\n");

	pthread_t tid;

	printf("Bouta create the pthread\n");
	pthread_create(&tid,NULL,myturn,NULL);
	//pthread_create(&tid,NULL,sum_runner,NULL);
	yourturn();
	pthread_join(tid,NULL);
	printf("Sum is %lld\n",sum);

	return 0;
}
