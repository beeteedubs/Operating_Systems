/* syscall.c
 *
 * Group Members Names and NetIDs:
 *   1. Bryan Zhu bjz20
 *   2. Ritvik Biswas rb1008
 *
 * ILab Machine Tested on: ILab 2
 *
 */
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <sys/syscall.h>

double avg_time = 0;

int main(int argc, char *argv[])
{
	 /* Implement Code Here */
	long num_iterations = 50000;
   	struct timeval curr_time;
	gettimeofday(&curr_time,NULL);
	int i = 0;
	for(i;i<num_iterations;i++){
		int fd = syscall(2,"test.text");	// open file
		syscall(3, fd);				// close file
	}    
	long total_time  = curr_time.tv_usec;
	float avg_time = total_time/num_iterations;
	printf("Average time per system call is %f microseconds\n",avg_time);
    	return 0;
}
