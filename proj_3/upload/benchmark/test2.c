#include "../my_vm.h"
#include <stdio.h>

int main(){
	set_physical_mem();

	unsigned int x = 4097;//1 page
	void* a = a_malloc(x);
//	a_free(a,4096);
	//unsigned long addr = 2147483648; // printf invalid cuz nothing at vpn 52...
	unsigned long virt_addr = 1<<13;// VPN 2
	virt_addr++;
	printf("virt_addr:%ld\n",virt_addr);

	unsigned long phys_addr = 1<<14;// PPN 4
	phys_addr++;
	printf("phys_addr:%ld\n",phys_addr);	

	unsigned long *ptr_addr = &virt_addr;

	int page_map_return = page_map(NULL,(void*)ptr_addr,&phys_addr);
	pte_t *ptr = translate(NULL,(void*)ptr_addr);
	printf("physical address: %ld",*ptr);
	return 0;
}
