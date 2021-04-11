#include "../my_vm.h"
#include <stdio.h>

int main(){
	testing();
/*
    int r = 3, c = 4;
    int *arr = (int *)malloc(r * c * sizeof(int));
  
    int i, j, count = 0;
    for (i = 0; i <  r; i++)
      for (j = 0; j < c; j++)
         *(arr + i*c + j) = ++count;
  
    for (i = 0; i <  r; i++)
      for (j = 0; j < c; j++)
         printf("%d ", *(arr + i*c + j));
*/  

	set_physical_mem();
	//unsigned long addr = 2147483648; // printf invalid cuz nothing at vpn 52...
	unsigned long virt_addr = 1<<13;
	unsigned long phys_addr = 1<<14;
	unsigned long *ptr_addr = &virt_addr;
	int page_map_return = page_map(NULL,(void*)ptr_addr,&phys_addr);
	pte_t *ptr = translate(NULL,(void*)ptr_addr);
	printf("physical address: %ld",*ptr);
	return 0;
}
