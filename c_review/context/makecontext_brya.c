#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <sys/types.h>
#include <unistd.h>
#include <ucontext.h>

#define STACK_SIZE SIGSTKSZ

void simplef(){
	puts("threaded\n");
}

int main(int argc, char **argv){
	ucontext_t cctx, nnctx;

	//init context, always do this before makecontext() when creating new context
	if (getcontext(&cctx) <0){
		perror("getcontext");
		exit(1);
	}

	void *stack = malloc(STACK_SIZE);

	cctx.uc_link = NULL;
	cctx.uc_stack.ss_sp=stack; //user space stack to save context
	cctx.uc_stack.ss_size=STACK_SIZE;
	cctx.uc_stack.ss_flags=0;

	puts("about to make context");

	//setup the context to run simplef
	makecontext(&cctx,(void*)&simplef,0);
	puts("Successfully modified context");

	setcontext(&cctx);
	
	return 0;
}
