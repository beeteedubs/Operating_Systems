#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
int main(){
	if (1>0){
		uint32_t v = 0xdeadbeef;
		printf("%02x\n",((char *)&v)[0]);
	}
	return 0;
}
