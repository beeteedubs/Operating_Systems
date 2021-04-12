#include "my_vm.h"

char* phys_mem;				// 1GB memory space to address 4GB
char* virt_bitmap;			// (required) each bit represents whether virtual page allocated or not
char* phys_bitmap;			// (required) each bit represents whether allocated or not

int num_addressing_bits=32;	// given in project description
int num_vpn_bits; 			// includes page directory and page table bits
int num_offset_bits; 		// bits for offset
int num_pd_bits; 			// bits for page directory
int num_pt_bits;			// bits for page table
int num_pde;
int num_pte;

int num_virt_pages;			// number of pages in fake physical memory
int num_phys_pages;			// number of pages in whole address space

pde_t* pd;					// pointer to PD, ** since it's array of arrays

bool is_phys_mem_init;		// set in a_malloc()/set_physical_mem()

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

void testing(){
	printf("working: able to obtain functions from my_vm lib\n");
	return;
}


/*Function that gets the next available page
*/
void *get_next_phys_avail(){
	
	int i = 0; // index of virtual pages (ex: i's range=[0,2^32-1])
	static int phys_start = -1; // start index of virtual pages

	while(i<num_phys_pages){ // while no starting point o
		if(get_bit_at_index(phys_bitmap,i) == 0){// it's free!
			phys_start = i;
			set_bit_at_index(phys_bitmap,i);
			break;
		}
		i++;
	}
	return &phys_start;

}

/*Helper functions regarding bit parsing*/
static unsigned long get_top_bits(unsigned long value,  int num_bits)
{
    //Assume you would require just the higher order (outer)  bits,
    //that is first few bits from a number (e.g., virtual address)
    //So given an  unsigned int value, to extract just the higher order (outer)  “num_bits”
    int num_bits_to_prune = 32 - num_bits; //32 assuming we are using 32-bit address
    return (value >> num_bits_to_prune);
}

static unsigned long get_mid_bits (unsigned long value, int num_middle_bits, int num_lower_bits)
{

   //value corresponding to middle order bits we will returning.
   unsigned long mid_bits_value = 0;   
    
   // First you need to remove the lower order bits (e.g. PAGE offset bits). 
   value =    value >> num_lower_bits; 


   // Next, you need to build a mask to prune the outer bits. How do we build a mask?   

   // Step1: First, take a power of 2 for “num_middle_bits”  or simply,  a left shift of number 1.  
   // You could try this in your calculator too.
   unsigned long outer_bits_mask =   (1 << num_middle_bits);  

   // Step 2: Now subtract 1, which would set a total of  “num_middle_bits”  to 1 
   outer_bits_mask = outer_bits_mask-1;

   // Now time to get rid of the outer bits too. Because we have already set all the bits corresponding 
   // to middle order bits to 1, simply perform an AND operation. 
   mid_bits_value =  value &  outer_bits_mask;

  return mid_bits_value;

}

static void set_bit_at_index(char *bitmap, int index)
{
    // We first find the location in the bitmap array where we want to set a bit
    // Because each character can store 8 bits, using the "index", we find which 
    // location in the character array should we set the bit to.
    char *region = ((char *) bitmap) + (index / 8);
    
    // Now, we cannot just write one bit, but we can only write one character. 
    // So, when we set the bit, we should not distrub other bits. 
    // So, we create a mask and OR with existing values
    char bit = 1 << (index % 8);

    // just set the bit to 1. NOTE: If we want to free a bit (*bitmap_region &= ~bit;)
    *region |= bit;
   
    return;
}

static int get_bit_at_index(char *bitmap, int index)
{
    //Same as example 3, get to the location in the character bitmap array
    char *region = ((char *) bitmap) + (index / 8);

    //Create a value mask that we are going to check if bit is set or not
    char bit = 1 << (index % 8);

    return (int)(*region >> (index % 8)) & 0x1;
}
void get_all_bits(char *bitmap){
	for(int i = 10;i>=0;i--){
		printf("%d",get_bit_at_index(bitmap,i));
	}
	printf("\n");
	return;
}
void clear_bit_at_index(char* bitArr, int k){
    bitArr[k/(sizeof(char)*8)] &= ~(1 << (k%(sizeof(char)*8)));
	return;
}
//boolean check_bitmap(char* bitmap, int index

//boolean check_bounds

//int get_vpn()
/////////////////////////////////////////////////////////////////////



/*
Function responsible for allocating and setting your physical memory 
*/
void set_physical_mem() {

    //Allocate physical memory using mmap or malloc; this is the total size of
    //your memory you are simulating

    
    //HINT: Also calculate the number of physical and virtual pages and allocate
    //virtual and physical bitmaps and initialize them

	/* 				GIVEN				*/
	
	// # offset bits
	// ex: 4096 bits/page -> 1000...000 (12 0 's) -> 12  offset bits = # of times >> until < 1
	int p = PGSIZE;
    num_offset_bits = 0;
    while (p > 1) {
        p>>=1;
        num_offset_bits++;
    }

	// ex: total_VPN_bits = 32 - 12 = 20 (ie 10 for PD and 10 PT)
	num_vpn_bits = num_addressing_bits - num_offset_bits;

	// # bits for in 32 bits for indexing in PD and PT
	// ex: 20 VPN bits -> split half for 2 level page tables
	num_pt_bits = num_vpn_bits/2;
	num_pd_bits = num_addressing_bits - num_offset_bits - num_pt_bits;

	// # of PTE and PDE
	// ex: 10 bits for page table, so 2^10 entries
	num_pte = 1<<num_pt_bits;
	num_pde = 1<<num_pd_bits;

	//  # of virtual and physical pages
	// ex: 4GB support, so 4GB/4KB = 2^20 pages 
	num_virt_pages = MAX_MEMSIZE/PGSIZE;
	num_phys_pages = MEMSIZE/PGSIZE;

	// fake physical memory
	phys_mem = (char*)malloc(sizeof(char)*MEMSIZE);

	// divide by 8 because chars are 8 bits, and each char holds 8 pages
	virt_bitmap = (char*) malloc( num_virt_pages / sizeof(char) );
    phys_bitmap = (char*) malloc( num_phys_pages / sizeof(char) );

    // initialize all bits = 0
    memset(virt_bitmap,0,num_virt_pages / sizeof(char));
    memset(phys_bitmap,0,num_phys_pages / sizeof(char));
	
	// indicate that PD and a page table takes  VPN 0 and VPN 1, so should be set
	///////////////////////UNSURE/////////////////////////////
	set_bit_at_index(virt_bitmap,0); 
//	set_bit_at_index(virt_bitmap,1); 

	// indicate that physical page number 0 and 1 taken by page directory and 1st page table
	set_bit_at_index(phys_bitmap,0); 
//	set_bit_at_index(phys_bitmap,1); 
	

	// reserve PD from phys_mem
	// cast to prevent warning and error
	pd = (unsigned long*) phys_mem;


	//testing purposes, set VPN2, offset 0 = Physical Frame Number 8
	/*pde_t pde_pfn = 1;
	pte_t pte_index = 2;
	pd[0] = pde_pfn; // 1st PDE points to Physical Frame Number 1, where page table stored 

	// go to PFN 1, just shift num_pt_bits to skip 4k bits 1x, that will contain actual physical addr
	unsigned long *page_table  = (unsigned long*)(phys_mem+(pde_pfn<<num_pt_bits));
	pte_t *pte_addr = page_table+pte_index*sizeof(pte_t); // get pte_addr

	pte_t actual_pa = 8;
	set_bit_at_index(phys_bitmap,actual_pa); 
	*pte_addr = actual_pa;*/

	is_phys_mem_init = true;
	return;

			
}


/*
 * Part 2: Add a virtual to physical page translation to the TLB.
 * Feel free to extend the function arguments or return type.
 */
int
add_TLB(void *va, void *pa)
{

    /*Part 2 HINT: Add a virtual to physical page translation to the TLB */

    return -1;
}


/*
 * Part 2: Check TLB for a valid translation.
 * Returns the physical page address.
 * Feel free to extend this function and change the return type.
 */
pte_t *
check_TLB(void *va) {

    /* Part 2: TLB lookup code here */

}


/*
 * Part 2: Print TLB miss rate.
 * Feel free to extend the function arguments or return type.
 */
void
print_TLB_missrate()
{
    double miss_rate = 0;	

    /*Part 2 Code here to calculate and print the TLB miss rate*/




    fprintf(stderr, "TLB miss rate %lf \n", miss_rate);
}



/*
The function takes a virtual address and page directories starting address and
performs translation to return the physical address
*/
pte_t *translate(pde_t *pgdir, void *va) {

    /* Part 1 HINT: Get the Page directory index (1st level) Then get the
    * 2nd-level-page table index using the virtual address.  Using the page
    * directory index and page table index get the physical address.
    *
    * Part 2 HINT: Check the TLB before performing the translation. If
    * translation exists, then you can return physical address from the TLB.
    */

	/*get VPN */
	unsigned long virtual_address =*((unsigned long*)(va)); // cast, then deref

    //break up into outer PN, inner PN, offset
    unsigned long pd_index = get_top_bits(virtual_address, num_pd_bits); //note: generalized get_top_bits(va,10)
    unsigned long pt_index = get_mid_bits(virtual_address, num_pt_bits, num_offset_bits);
    unsigned long offset = get_mid_bits(virtual_address, num_offset_bits, 0);

    //VPN = page directory index * number of entries in page table + page table index
	unsigned long vpn  = pd_index * num_pte + pt_index; // 2*1024+5 = ...

	// check VPN = textbook solution = (virtual address & vpn mask) >> shift (TO DO LATER)
	//unsigned long *vpn_mask;
	//memset(vpn_mask,0,4);
	//unsigned long vpn_check = virtual_address & vpn_mask >> shift

	// if VPN not in bounds, return
	if(!((vpn < num_virt_pages) && (vpn>0))){
		printf("VPN: %lu is out of bounds\n",vpn);
		return NULL;
	}
	
	// if VPN is invalid, return
	if(get_bit_at_index(virt_bitmap,vpn)==0){
		printf("VPN: %lu is invald\n",vpn);
		return NULL;
	}

	/*find and check if PDE contains a valid physical frame number (ie there's a page table at this physical address w/ at least 1 valid PTE) in physical memory*/
	// find PDE = index into pd pointer
	get_all_bits(phys_bitmap);
	get_all_bits(virt_bitmap);
	pde_t pde_pfn = pd[pd_index]; // pointer since pd is **, and points to first PDE in page table
	if(get_bit_at_index(phys_bitmap,pde_pfn)==0){// whole entry is PFN
		printf("pde points to invaliiid physical page # according to phys_bitmap\n");
		return NULL;
	}
	/*find and check if PTE valid and return physical address*/

	//find PTE= pde.pfn + pt_index*sizeof(pte)
	pte_t *page_table = (unsigned long*)(phys_mem+(pde_pfn<<num_pt_bits));
	pte_t *pte_addr = page_table+pt_index*sizeof(pte_t);

	//double-check that it's valid and return
	printf("PFN at pte: %lu,\n",*pte_addr);
	*pte_addr=((*pte_addr)*PGSIZE)+offset;
	printf("pa: %lu\n",*(pte_addr));
	return pte_addr;
	
}


/*
The function takes a page directory address, virtual address, physical address
as an argument, and sets a page table entry. This function will walk the page
directory to see if there is an existing mapping for a virtual address. If the
virtual address is not present, then a new entry will be added
*/
int page_map(pde_t *pgdir, void *va, void *pa)
{

    /*HINT: Similar to translate(), find the page directory (1st level)
    and page table (2nd-level) indices. If no mapping exists, set the
    virtual to physical mapping */

	/* check if existing mapping using virt_bitmap and extracting VPN*/

	// get those 32 bits
    unsigned long virtual_address =*((unsigned long*)(va));
	unsigned long physical_address =*((unsigned long*)(pa));

    //break up into outer PN, inner PN, offset

	// ex: pd_bits will provide the index (ie 12 for PDE at index 12)
    unsigned long pd_index = get_top_bits(virtual_address, num_pd_bits); //note: generalized get_top_bits(va,10)
    unsigned long pt_index = get_mid_bits(virtual_address, num_pt_bits, num_offset_bits);
    unsigned long offset = get_mid_bits(virtual_address, num_offset_bits, 0);

	unsigned long physical_pn = get_mid_bits(physical_address,30-num_offset_bits,num_offset_bits);//18 mid bits after offset (ignore 1st 2 bits)
    //get VPN
	unsigned long vpn  = pd_index * num_pte + pt_index;
	
	// if VPN not in bounds, return
	if(!((vpn < num_virt_pages) && (vpn>0))){
		printf("VPN: %ld is out of bounds\n",vpn);
		return -1;
	}

	// if VPN is invalid, return
	if(get_bit_at_index(virt_bitmap,vpn)==0){
		printf("VPN: %lu is invald\n",vpn);
		return -1;
	}
	
	// if VPN and Physical PN invalid, set to be valid cuz we creating the mapping now boyyyy
	if(get_bit_at_index(phys_bitmap,physical_pn)==0){
		printf("VPN: %ld is valid,creating mapping\n",vpn);
		
		// this could be invalid, meaning we need a new page table
		if((pd[pd_index]) == 0){// get PFN to put new page table
			pd[pd_index] = *((pte_t*)(get_next_phys_avail())); //ex: next free PFN was 101 
		}

		// get physical frame number, which gives location of page table
		pde_t pde_pfn = pd[pd_index];

		// get page table w/ physical frame number
		unsigned long *page_table = (unsigned long*)(phys_mem+(pde_pfn<<num_pt_bits));

		//get PTE address by indexing down the page table
		pte_t *pte_addr = page_table + pt_index*sizeof(pte_t);
	
		// set physical address in pte;
		*pte_addr = physical_pn;

		//change phys_bitmap
		set_bit_at_index(virt_bitmap,vpn);
		set_bit_at_index(phys_bitmap,physical_pn);
	}else{
		printf("already entry here...wait isn't that bad?\n");
		return -1;
	}

	/*map*/


    return 1;
}


void *get_next_avail(int num_pages) {
	/*
	   - find num_pages of free+contiguous virtual  pages in virt_bitmap (ex: find 2 pages for 4076 B)
	   - (not part of this fxn) finding num_pages of in phys_memory is trivial since they don't have to be contiguous

	   loop i in bitmap and until variable virt_start FN is not -1
	   		if virt_bitmap at index i is 1
				virt_start = i // or PhysicalPN
				loop num_pages using i
					if get_bit == 0 ever
						set virt_start=-1 // restart :(
		if virt_start != -1
			return &virt_start
	*/
	
	int i = 0; // index of virtual pages (ex: i's range=[0,2^32-1])
	static int virt_start = -1; // start index of virtual pages
	/*
	edge cases:
	- ez: all 0s
	- hard: last num_page bits r 0s
	- hard: (no solution) no bits r 0s
	- hard: (no solution) only last bit 0
	*/
	while(i+num_pages<=num_virt_pages && virt_start == -1){ // while no starting point o
		if(get_bit_at_index(virt_bitmap,i) == 0){// it's free!
			virt_start = i;
			int count = 1;
			while(count<num_pages){//start at 1, since already counting first page
				if(get_bit_at_index(virt_bitmap,i)==1){//restart!
					virt_start = -1;
					break;
				}
				i++;
				count++;
			}
			if(virt_start != -1){
				return &virt_start;
			}
		}else{
			i++;
		}
	}
	return &virt_start;
}


/* Function responsible for allocating pages
and used by the benchmark
*/
void *a_malloc(unsigned int num_bytes) {
	printf("a_mallocing------------\n");
    /* 
     * HINT: If the physical memory is not yet initialized, then allocate and initialize.
     */

	/* 
    * HINT: If the page directory is not initialized, then initialize the
    * page directory. Next, using get_next_avail(), check if there are free pages. If
    * free pages are available, set the bitmaps and map a new page. Note, you will 
    * have to mark which physical pages are used. 
    */
	pthread_mutex_lock(&mutex);

	if(is_phys_mem_init == false){
		set_physical_mem();
		is_phys_mem_init = true;
	}
	// get num_pages
	int num_pages = num_bytes / PGSIZE;
	if(num_bytes % PGSIZE !=0){
		num_pages +=1;
	}

	// get ptr to virtual address from VPN
	unsigned long vpn = *(unsigned long*)(get_next_avail(num_pages));
	if(vpn!=-1){//-1 means no pages left
		printf("found some pages, setting the valid so they can be mapped\n");
		for(int i=0;i<num_pages;i++){
			set_bit_at_index(virt_bitmap,vpn+i);
		}
	}else{
		printf("oops no pages contiguous pages left\n");
		return NULL;
	}

	// start mapping!

	unsigned long virtual_address = vpn*PGSIZE;//remember no offset here
	unsigned long* va_ptr = &virtual_address; // return this
	unsigned long* pa;
	int count = 0;
	unsigned long page_map_va_ptr; //change later
	while(count<num_pages){
		pa = (unsigned long*)(get_next_phys_avail());
		set_bit_at_index(phys_bitmap,*pa);
		*pa = (*pa)*PGSIZE;
		page_map_va_ptr =(count+vpn)*PGSIZE;
		int ret_val = page_map(NULL,(void*)(&page_map_va_ptr),(void*)(pa));//don't need to use counter for ppn 
		if(ret_val==1){//success
			printf("success\n");
		}
		else{
			printf("err: failed....\n");
			return NULL;
		}
		count++;
	}
	pthread_mutex_unlock(&mutex);
	return (void*)va_ptr;
}


/* Responsible for releasing one or more memory pages using virtual address (va)
*/
void a_free(void *va, int size) {

    /* Part 1: Free the page table entries starting from this virtual address
     * (va). Also mark the pages free in the bitmap. Perform free only if the 
     * memory from "va" to va+size is valid.
     * Part 2: Also, remove the translation from the TLB
     */
	//get vpn
    unsigned long virtual_address =*((unsigned long*)(va));
    unsigned long pd_index = get_top_bits(virtual_address, num_pd_bits); //note: generalized get_top_bits(va,10)
    unsigned long pt_index = get_mid_bits(virtual_address, num_pt_bits, num_offset_bits);
    unsigned long offset = get_mid_bits(virtual_address, num_offset_bits, 0);
    //get VPN
	unsigned long vpn  = pd_index * num_pte + pt_index;
	//set bit to 0
	//get num pages
	int num_pages = (int)ceil((double)(size/PGSIZE));
	// just set bitmap bits to 0
	for(int i=0;i<num_pages;i++){
		if(get_bit_at_index(virt_bitmap,vpn+i)==0){
			printf("err:this was alredy unused\n");
			return;
		}
		pte_t pa_or_pfn = *(translate(NULL,va + (i*PGSIZE)));
		if(pa_or_pfn==0){
			printf("err: physical addr is null\n");
			return;
		}
		clear_bit_at_index(virt_bitmap,vpn+i);
		clear_bit_at_index(phys_bitmap,pa_or_pfn);
	}
	printf("success\n");
    return; 
}


/* The function copies data pointed by "val" to physical
 * memory pages using virtual address (va)
*/
void put_value(void *va, void *val, int size) {

    /* HINT: Using the virtual address and translate(), find the physical page. Copy
     * the contents of "val" to a physical page. NOTE: The "size" value can be larger 
     * than one page. Therefore, you may have to find multiple pages using translate()
     * function.
     */


	return;

}


/*Given a virtual address, this function copies the contents of the page to val*/
void get_value(void *va, void *val, int size) {

    /* HINT: put the values pointed to by "va" inside the physical memory at given
    * "val" address. Assume you can access "val" directly by derefencing them.
    */


	return;

}



/*
This function receives two matrices mat1 and mat2 as an argument with size
argument representing the number of rows and columns. After performing matrix
multiplication, copy the result to answer.
*/
void mat_mult(void *mat1, void *mat2, int size, void *answer) {

    /* Hint: You will index as [i * size + j] where  "i, j" are the indices of the
     * matrix accessed. Similar to the code in test.c, you will use get_value() to
     * load each element and perform multiplication. Take a look at test.c! In addition to 
     * getting the values from two matrices, you will perform multiplication and 
     * store the result to the "answer array"
     */
	return ;
       
}
