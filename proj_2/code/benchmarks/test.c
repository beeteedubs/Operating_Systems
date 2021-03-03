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
long long sum = 0;

void* sum_runner(void *arg){
	long long *limit_ptr = (long long*) arg;
	long long limit = *limit_ptr;

	for (long long i= 0; i <=limit; i++){
		sum += i;
	}
	pthread_exit(0);
}
int main(int argc, char **argv) {

	if (argc < 2){
			printf("Usage: %s <num>\n",argv[0]);
			exit(-1);
	}

	long long limit = atoll(argv[1]);

	pthread_t tid;

	pthread_attr_t attr;
	//pthread_attr_init(&attr);

	pthread_create(&tid,NULL,sum_runner,&limit);

	pthread_join(tid,NULL);
	printf("Sum is %lld\n",sum);

	return 0;
}
