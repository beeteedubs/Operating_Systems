#include <stdlib.h>
#include <stdio.h>
void foo(int * p1){
	int num = (int)(*p1);
	printf("num: %d\n",num);
	return;
}

void foo2(char *arr){

	int* ptr2 = (int*)arr;
	printf("ptr2[0]: %d\n",ptr2[0]);
	return;
}

int main(){
	/*int a = 5;
	foo(&a);*/
	char *ptr = (char*)malloc(8*sizeof(char));
	for(int i=0;i<8;i++){
		ptr[i] = 'a';
	}
	for(int j=0;j<8;j++){
		printf("ptr[%d]:%d\n",j,ptr[j]);
	}
	foo2(ptr);
	return 0;
}
