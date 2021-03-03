#include "basics.h"

#define STACK_SIZE SIGSTKSZ
unsigned ctx_id = 0;
ucontext_t ctx[2];

void foo(){
	printf("FOO starting!!!\n");
	while(1){
		puts("foo");
		usleep(100000);//0.1 sec
	}
}

void bar(){
	printf("BAR starting!!!\n");
	while(1){
		puts("bar");
		usleep(100000);
	}
}

void signal_handler(int signum){
	printf("SWITCH\n");
	unsigned ctx_cur = ctx_id; //ctx_cur = 0
	unsigned ctx_next = (ctx_id + 1) & 0x01; //ctx_next =  1
	ctx_id = (ctx_id + 1) & 0x01; //ctx_id = 1
	swapcontext(&ctx[ctx_cur], &ctx[ctx_next]);
}

int main(int argc, char** argv){
	// init context
	char stack[2][STACK_SIZE];
	void (*func[2])() = {foo, bar};
	ucontext_t m_ctx;

	for (unsigned c=0; c<2;c++){
		unsigned c_next = (c+1) & 0x1;
		memset(&ctx[c], 0, sizeof(ctx[c])); //clear out context w/ 0's
		getcontext(&ctx[c]);
		ctx[c].uc_stack.ss_sp = stack[c];
		ctx[c].uc_stack.ss_size = STACK_SIZE;
		ctx[c].uc_link = NULL;// is this essential?
		makecontext(&ctx[c], func[c],0);
	}
	// init sig
	struct sigaction sa;
	memset(&sa, 0, sizeof(sa));
	sa.sa_handler = &signal_handler;
	sigaction(SIGPROF,&sa, NULL);

	// init timer
	struct itimerval timer;
	// run signal every 1 second
	timer.it_interval.tv_usec = 0;
	timer.it_interval.tv_sec = 1;
	// start timer at
	timer.it_value.tv_usec=0;
	timer.it_value.tv_sec=1;
		
	// enable timer
	setitimer(ITIMER_REAL,&timer,NULL);
	
	// never going back to m_ctx so don't need inf loop
	setcontext(&ctx[0]);
	//swapcontext(&m_ctx,&ctx[0]);
	return 0;
}
