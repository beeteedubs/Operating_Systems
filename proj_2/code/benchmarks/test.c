#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include "../rpthread.h"


/* A scratch program template on which to call and
 * test rpthread library functions as you implement
 * them.
 *
 * You can modify and use this program as much as possible.
 * This will not be graded.
 */
long long sum1 = 0;
long long sum2 = 0;

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

void* sum_runner1(void* args){
	printf("In sum_runner!\n");

	for (long long i= 0; i <=90; i++){
		sum1 += i;
	}
	//rpthread_yield();
	printf("Sum 1 is %d, FINISHED SR1\n\n\n", sum1);

	pthread_exit(0);
}

void* sum_runner2(){
        printf("In sum_runner 2!\n");
        //long long *limit_ptr = (long long*) arg;
        //long long limit = *limit_ptr;

        for (long long i= 0; i <=5; i++){
                sum2 += i;
        }
	//rpthread_yield();
        printf("Sum 2 is %d\n", sum2);
        pthread_exit(0);
}


int main(int argc, char **argv) {
	printf("In main!\n");

	pthread_t tid;

	printf("Bouta create the pthread\n");

	pthread_create(&tid,NULL,sum_runner1,NULL);

	pthread_create(&tid, NULL, sum_runner2,NULL);
	pthread_join(tid,NULL);
	printf("Sum 1 is %lld\n",sum1);
	//rpthread_yield();
	//rpthread_yield();
	printf("Main done\n");
	//pthread_create(&tid,NULL,myturn,NULL);
	//pthread_create(&tid,NULL,sum_runner,NULL);
	//yourturn();
	//pthread_join(tid,NULL);
	//printf("Sum is %lld\n",sum);


	return 0;
}
