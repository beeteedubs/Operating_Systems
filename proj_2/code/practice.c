#include "basics.h"

#define STACK_SIZE SIGSTKSZ

void foo(ucontext_t *bctx){
	for (;;){
		puts("foo");
		sleep(2);
		setcontext(bctx);
	}
}

void bar(ucontext_t *mctx){
	for(;;){
		puts("bar");
		sleep(2);
		setcontext(mctx);
	}
}

int main(int argc, char** argv){
	/* STEPS
		- init 
			- sigaction to reigster signal handler

	*/
	//nctx = main's context
	ucontext_t fctx,bctx,mctx;

	// init contexts to this context, make them into foo and bar contexts later
	getcontext(&fctx);
	getcontext(&bctx);
	getcontext(&mctx);

	// do this cuz makecontext() requires this
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
	makecontext(&fctx,(void *)&foo,1,&bctx);
	makecontext(&bctx,(void *)&bar,1,&mctx);
	puts("successfully modified context");

	// setcontext(), no need to swap and save here
	swapcontext(&mctx,&fctx);// start w/ foo
	
	// test for error
	puts("this should run after foo and bar printed");
	
	return 0;
}
