#include "../basics.h"

void Double(int A[], int size){
	
	//int sum = 0;
	for(int i = 0; i < size; i++){
		//printf("*(A+i): %d\n",*(A+i));
		*(A+i)=2*(*(A+i));
	}
	//printf("Sum: %d\n",sum);
	return;
}
		

int main(int argc, char** argv){
	int A[] = {1,2,3,4,5};
	int size = sizeof(A)/sizeof(A[0]);
	Double(A,size);
	for(int i = 0; i<size;i++){
		printf("A[%d]: %d\n",i,A[i]);
	}
	return 0;
}
