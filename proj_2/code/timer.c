#include "basics.h"

void ring(int signum){
	puts("timer gone off");
}

int main(){
	struct sigaction sa;
	memset(&sa,0,sizeof(sa)); // inits to all 0s?
	sa.sa_han
}
