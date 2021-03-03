#include <signal.h>
#include <sys/time.h>
#include <ucontext.h>
#include <stdio.h> // printf
#include <string.h>
#include <time.h>
#include <unistd.h>

static unsigned ctx_id = 0;
static ucontext_t ctx[4];


void signal_action(int signal){
	printf("SWITCH\n");
	unsigned ctx_cur = ctx_id;
	unsigned ctx_next = (ctx_id+1) & 0x03;
	ctx_id = (ctx_id + 1) & 0x03;
	if(swapcontext(&ctx[ctx_cur], &ctx[ctx_next]) == -1){
		printf("ERROR\n");
	}
}
static void func1(void){
	printf("func1: starting\n");
	while(1){
		printf("func1\n");
		usleep(100000);
	}
}
static void func2(void){
	printf("func2: starting\n");
	while(1){
		printf("func2\n");
		usleep(100000);
	}
}
static void func3(void){
	printf("func3: starting\n");
	while(1){
		printf("func3\n");
		usleep(100000);
	}
}
static void func4(void){
	printf("func4: starting\n");
	while(1){
		printf("func4\n");
		usleep(100000);
	}
}
int main(){
	static const unsigned STACK_SIZE = 16384;
	char stack[4][STACK_SIZE];
	void (*func[4])() = {func1, func2, func3, func4};
	ucontext_t ctx_main;

	for (unsigned c=0; c<4;c++){
		unsigned c_next = (c+1) & 0x3;
		memset(&ctx[c], 0, sizeof(ctx[c])); //clear out context w/ 0's
	 	if (getcontext(&ctx[c]) == -1){
			printf("failed get context\n");
		}
		ctx[c].uc_stack.ss_sp = stack[c];
		ctx[c].uc_stack.ss_size = STACK_SIZE;
		ctx[c].uc_link = NULL;// is this essential?
		makecontext(&ctx[c], func[c],0);
	}
	// init sig
	struct sigaction sa;
	memset(&sa, 0, sizeof(sa));
	sa.sa_handler = &signal_action;
	sigaction(SIGALRM, &sa, NULL);

	// init timer
	struct itimerval timer;
	timer.it_interval.tv_usec = 0;
	timer.it_interval.tv_sec = 1; // must be !=0, else one-shot timer
	timer.it_value.tv_usec=0;
	timer.it_value.tv_sec=1; // must be set to 1, or else won't start
		
	// enable timer
	setitimer(ITIMER_REAL,&timer,NULL);
	swapcontext(&ctx_main, &ctx[0]);
	return 0;
}
