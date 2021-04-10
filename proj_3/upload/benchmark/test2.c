#include "../my_vm.h"
#include <stdio.h>

int main(){
	testing();
	set_physical_mem();
	//unsigned long addr = 2147483648; // printf invalid cuz nothing at vpn 52...
	unsigned long addr = 1<<12;
	unsigned long *ptr_addr = &addr;
	pte_t *ptr = translate(NULL,(void*)ptr_addr);
	return 0;
}
