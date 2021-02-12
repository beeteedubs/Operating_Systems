#include <signal.h>
#include <stdio.h>
#include <stdlib.h>

void segment_fault_handler(int signum){
	printf("I am slain!\n");
    	int *ptr = &signum + 51;//51*4= 204 bytes to reach to saved rip in stack 2
       	*ptr += 5; // 5 bytes to skip to next instruction
	return;
}

int main(int argc, char *argv[]){
    int r2 = 0;
    signal(SIGSEGV,segment_fault_handler);
    r2 = *((int *)0); // This will generate segmentation fault
    printf("I live again!\n");
    return 0;
}
