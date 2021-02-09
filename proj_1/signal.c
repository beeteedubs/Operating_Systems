#include <signal.h>
#include <stdio.h>
#include <stdlib.h>

void segment_fault_handler(int signum){
	printf("I am slain!\n");
    	int *ptr = &signum + 51;//7*4= 28 bytes
       	*ptr += 5; // 10 bytes -> illegal instruction
//ptr += 0xd1c;
	return;
}

int main(int argc, char *argv[]){
    int r2 = 0;
    signal(SIGSEGV,segment_fault_handler);
    r2 = *((int *)0); // This will generate segmentation fault
    printf("I live again!\n");
    return 0;
}
