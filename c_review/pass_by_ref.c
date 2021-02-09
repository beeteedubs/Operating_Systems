#include <stdint.h>

void foo(int **a, int**b){
	*a=*b;
}

int main(){
	int z = 1;
	int *b = &z;
	int **a = &b;
	printf("z: %x, %d\n",&z, a);
	printf("b: %x, %x\n",&b, b);
	printf("a: %x, %x\n", &a, a);
	foo(a,&b);
	printf("a: %x, %x\n",&a,a);
	return 0;
}
