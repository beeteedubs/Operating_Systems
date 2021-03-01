#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <sys/types.h>
#include <unistd.h>
#include <ucontext.h>

#define STACK_SIZE SIGSTKSZ

int main(int argc, char **argv){
	ucontext_t cctx, nnctx;

	if (argc != 1){
		printf("no args expected\n");
		exit(1);
	}

	//init context, always do this before makecontext() when creating new context
	if (getcontext(&cctx) <0){
		perror("getcontext");
		exit(1);
	}

	puts(" I am going again w/ no context\n");

	if (setcontext(&cctx) <0){
		perror("set curr context");
		exit(1);
	}
	puts(" LOoks like I am lost \n");
	return 0;
}
