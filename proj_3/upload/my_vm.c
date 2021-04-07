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

pde_t** pd;					// pointer to PD, ** since it's array of arrays


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

//traboolean check_bitmap(char* bitmap, int index

//boolean check_bounds
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
	
	// indicate that PD takes first VPN, so should be set
	set_bit_at_index(virt_bitmap,0); 

	// reserve PD from phys_mem
	// cast to prevent warning and error
	pd = (unsigned long *) phys_mem;
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
//pte_t *translate(pde_t *pgdir, void *va) {
pte_t *translate(pde_t *pgdir, unsigned long  *va) {

    /* Part 1 HINT: Get the Page directory index (1st level) Then get the
    * 2nd-level-page table index using the virtual address.  Using the page
    * directory index and page table index get the physical address.
    *
    * Part 2 HINT: Check the TLB before performing the translation. If
    * translation exists, then you can return physical address from the TLB.
    */

	/* 				GIVEN 			*/

	// get those 32 bits
    unsigned long virtual_address = (unsigned long)(*va);

    //break up into outer PN, inner PN, offset

	// ex: pd_bits will provide the index (ie 12 for PDE at index 12)
    unsigned long pd_index = get_top_bits(virtual_address, num_pd_bits); //note: generalized get_top_bits(va,10)
    unsigned long pt_index = get_mid_bits(virtual_address, num_pt_bits, num_offset_bits);
    unsigned long offset = get_mid_bits(virtual_address, num_offset_bits, 0);

    //get VPN
	unsigned long vpn  = pd_index * num_pte + pt_index;

	// if VPN not in bounds, return
	if(!((vpn < num_virt_pages) && (vpn>0))){
		printf("VPN: %ld is out of bounds\n",vpn);
		return NULL;
	}
	
	// if VPN is invalid, also return
	if(get_bit_at_index(virt_bitmap,vpn)==0){
		printf("VPN: %ld is invalid\n",vpn);
		return NULL;
	}

	pde_t* page_table = pd[pd_index]

	// confirmed, but double check PD has the PT
	//////////////////NOT SURE ABT THIS//////////////////////
	if(page_table == NULL){// get_bit_at_index(phys_bitmap,pd[pd_index])==0 could also work? 
		printf("Physical in the PDE hasn't been set yet\n");
		return NULL;
	}

	pte_t *phys_addr = &(page_table[pt_index]);


	// phys_mem < phys_addr < phys_mem+memsize
	//////////////////NOT SURE ABT THIS//////////////////////
	if( (*phys_addr<(unsigned long) phys_mem) || (*phys_addr > (unsigned long)phys_mem) + MEMSIZE){
		printf("physical address is not w/in bounds of MEMSIZE\n"); // needs to be MEMSIZE not MAX_MEMSIZE
		return NULL;
	}

	*phys_addr+=offset;

	return phys_addr;
}


/*
The function takes a page directory address, virtual address, physical address
as an argument, and sets a page table entry. This function will walk the page
directory to see if there is an existing mapping for a virtual address. If the
virtual address is not present, then a new entry will be added
*/
int
page_map(pde_t *pgdir, void *va, void *pa)
{

    /*HINT: Similar to translate(), find the page directory (1st level)
    and page table (2nd-level) indices. If no mapping exists, set the
    virtual to physical mapping */

    return -1;
}


/*Function that gets the next available page
*/
void *get_next_avail(int num_pages) {
 
    //Use virtual address bitmap to find the next free page
}


/* Function responsible for allocating pages
and used by the benchmark
*/
void *a_malloc(unsigned int num_bytes) {

    /* 
     * HINT: If the physical memory is not yet initialized, then allocate and initialize.
     */

   /* 
    * HINT: If the page directory is not initialized, then initialize the
    * page directory. Next, using get_next_avail(), check if there are free pages. If
    * free pages are available, set the bitmaps and map a new page. Note, you will 
    * have to mark which physical pages are used. 
    */

    return NULL;
}

/* Responsible for releasing one or more memory pages using virtual address (va)
*/
void a_free(void *va, int size) {

    /* Part 1: Free the page table entries starting from this virtual address
     * (va). Also mark the pages free in the bitmap. Perform free only if the 
     * memory from "va" to va+size is valid.
     *
     * Part 2: Also, remove the translation from the TLB
     */
     
    
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




}


/*Given a virtual address, this function copies the contents of the page to val*/
void get_value(void *va, void *val, int size) {

    /* HINT: put the values pointed to by "va" inside the physical memory at given
    * "val" address. Assume you can access "val" directly by derefencing them.
    */




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

       
}



