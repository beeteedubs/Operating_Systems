#include "basics.h"
int num_rings = 0;
void ring(int signum){
	num_rings = num_rings + 1;
	puts("timer gone off");
	while(1){
		printf(" %d\n",num_rings);
		sleep(1);
	}
}

int main(){
	struct sigaction sa;
	memset(&sa,0,sizeof(sa)); // inits to all 0s?
	sa.sa_handler = &ring; // set handler fxn to ring
	sigaction(SIGPROF,&sa,NULL); // register sighanlder

	// now timer stuff
	struct itimerval timer;

	// set what reset timer to be, if 0.0 then one-shot
	timer.it_interval.tv_usec =0;
	timer.it_interval.tv_sec=3;

	// set init value of timer of 1 second, if both 0 never runs
	timer.it_value.tv_usec = 0;
	timer.it_value.tv_sec = 3;

	// ITIMER_PROF, only counts when process is running or system runs for process
	setitimer(ITIMER_PROF,&timer,NULL);

	while(1);

	return 0;
}
