#include "basics.h"

#define STACK_SIZE SIGSTKSZ
int running = 0;
ucontext_t fctx,bctx;

void foo(){
	while(1){
		puts("foo");
	}
}

void bar(){
	while(1){
		puts("bar");
	}
}

void ring(int signum){
	puts("ring");
	if(running==0){//run foo
		running = 1;
		puts("runing =- 1");
		setcontext(&fctx);
	}else{ //run bar
		running = 0;
		puts("running = 0");
		setcontext(&bctx);
	}
}
int main(int argc, char** argv){

	// init sig
	struct sigaction sa;
	memset(&sa, 0, sizeof(sa));
	sa.sa_handler = &ring;
	sigaction(SIGPROF,&sa, NULL);

	// init timer
	struct itimerval timer;
	timer.it_interval.tv_usec = 0;
	timer.it_interval.tv_sec = 2;
	timer.it_value.tv_usec=0;
	timer.it_value.tv_sec=2;
	
	// init contexts
	getcontext(&fctx);
	getcontext(&bctx);

	// makecontext() requires this
	void *foo_stack = malloc(STACK_SIZE);
	fctx.uc_link = NULL;
	fctx.uc_stack.ss_sp=foo_stack;
	fctx.uc_stack.ss_size=STACK_SIZE;
	fctx.uc_stack.ss_flags=0;

	void *bar_stack = malloc(STACK_SIZE);
	bctx.uc_link = NULL;
	bctx.uc_stack.ss_sp=bar_stack;
	bctx.uc_stack.ss_size=STACK_SIZE;
	bctx.uc_stack.ss_flags=0;

	// make the contexts
	puts("about to make 2 contexts");
	makecontext(&fctx,(void *)&foo,0);
	makecontext(&bctx,(void *)&bar,0);
	puts("successfully modified context");

	// enable timer
	setitimer(ITIMER_PROF,&timer,NULL);
	
	// kill time, initially print nothing
	while(1);

	return 0;
}
