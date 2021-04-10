#include <stdio.h>

int *add(int x){
	int i = x+5;
	int *ptr = &i;
	return ptr;
}

int main(){

	int a = 5;
	int *ptr2 = add(a);
	printf("ptr2: %d\n",*ptr2);
	return 0;
}
