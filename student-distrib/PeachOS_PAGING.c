#include "types.h"
#include "PeachOS_PAGING.h"

#define VIDEO 0xB8000

page_directory_t page_directory[ONE_K] __attribute__((aligned(FOUR_K)));
page_table_t page_table[ONE_K] __attribute__((aligned(FOUR_K)));
page_table_t video_page_table[ONE_K] __attribute__((aligned(FOUR_K)));


/*****************************************************************
 *
 * paging_init
 *  DESCRIPTION:
 *          This function Initializes the page_directory and enables paging
 *  INPUT: none
 *  OUTPUT: none
 *  SOURCE: http://wiki.osdev.org/Paging#Page_Table
 *          https://courses.engr.illinois.edu/ece391/secure/references/IA32-ref-manual-vol-3.pdf
 *			Helpful pages: 3-22 to 3-28
 *
****************************************************************/
void
paging_init(void)
{
	pageDirectory_init(); //map 4Gb with 1024 entries in page table
	enablePaging();		  //enable paging registers
}

/*****************************************************************
 *
 * pageDirectory_init
 *  DESCRIPTION:
 *          This function is responsbile for creating the page directory and mapping
 * 			the kernel and video memory. It also creates the page table for addresses 0-4MB.
 *  INPUT: none
 *  OUTPUT: none
 *  SIDE EFFECTS: Fills page_directory and page_table structs
 *  SOURCE: http://wiki.osdev.org/Paging#Page_Table
 *          https://courses.engr.illinois.edu/ece391/secure/references/IA32-ref-manual-vol-3.pdf
 *			Helpful pages: 3-22 to 3-28
 *
****************************************************************/
void pageDirectory_init()
{
	int i;
	for(i=0; i<ONE_K; i++)
	{
		page_table[i].PBA = ((i*FOUR_K)>>12); // init first page table (0-4MB)
	}
	page_directory[0].PTBA = (((uint32_t)page_table)>>12); //init first page directory entry (0-4MB)
	page_directory[0].read_write = 1;
	page_directory[0].present = 1;
	page_directory[1].PTBA = FOUR_K/4;		//address of kernel 4MB page (4-8MB)
	page_directory[1].page_size = 1; 		//4MB page, not 4KB
	page_directory[1].user_supervisor = 0;	//supervisor mode set
	page_directory[1].read_write = 1;		//read / write enabled
	page_directory[1].present = 1;			//kernel (4-8MB) present
	page_table[VIDEO>>12].read_write = 1;	//video memory needs to be accessible
	page_table[VIDEO>>12].present = 1;
}

/*****************************************************************
 *
 * enablePaging
 *  DESCRIPTION:
 *          This function enables paging in the kernel.
 *  INPUT: page_directory
 *  OUTPUT: none
 *  SIDE EFFECTS: Sets cr0 and cr4, stores page_directory address in cr3
 *  SOURCE: http://wiki.osdev.org/Paging#Page_Table
 *          https://courses.engr.illinois.edu/ece391/secure/references/IA32-ref-manual-vol-3.pdf
 *			Helpful pages: 3-22 to 3-28
 *
****************************************************************/
void enablePaging()
{
	asm volatile("				\n\
		movl %0, %%eax 			\n\
		movl %%eax, %%cr3		\n\
		movl %%cr4, %%eax		\n\
		orl $0x00000010, %%eax 	\n\
		movl %%eax, %%cr4		\n\
		movl %%cr0, %%eax		\n\
		orl $0x80000000, %%eax	\n\
		movl %%eax, %%cr0		\n\
		"
		:                        /* no outputs */
		: "r"(page_directory)    /* input */
		: "eax"                  /* clobbered register */
		);
}

/*****************************************************************
 *
 * 	init_page
 *  DESCRIPTION:
 *          This function initializes a new Page for a new process
 *  INPUTs: a virtual address and a physical address (passed by reference)
 *  OUTPUT: none
 *  SIDE EFFECTS: Sets up a page directory in memory
 *  SOURCE: http://wiki.osdev.org/Paging#Page_Table
 *          https://courses.engr.illinois.edu/ece391/secure/references/IA32-ref-manual-vol-3.pdf
 *					Helpful pages: 3-22 to 3-28
 *
****************************************************************/

void init_page (uint32_t va, uint32_t pa)
{
	uint32_t PD_index = (va) >> PDBITSH;

	page_directory[PD_index].PTBA = (pa >> PTBITSH);
	page_directory[PD_index].available = 0;
	page_directory[PD_index].reserved = 0;
	page_directory[PD_index].accessed = 0;
	page_directory[PD_index].cache_disabled = 0;
	page_directory[PD_index].write_through = 0;
	page_directory[PD_index].user_supervisor = 1;
	page_directory[PD_index].page_size = 1;
	page_directory[PD_index].global_page = 0;
	page_directory[PD_index].read_write = 1;
	page_directory[PD_index].present = 1;

	flush_tlb();
}

/*****************************************************************
 *
 * 	init_set_page
 *  DESCRIPTION:
 *          This function initializes a new Page for a new process
 *  INPUTs: a virtual address and a physical address (passed by reference)
 *  OUTPUT: none
 *  SIDE EFFECTS: Sets up a page directory in memory
 *  SOURCE: http://wiki.osdev.org/Paging#Page_Table
 *          https://courses.engr.illinois.edu/ece391/secure/references/IA32-ref-manual-vol-3.pdf
 *					Helpful pages: 3-22 to 3-28
 *
****************************************************************/
void init_set_page (uint32_t va, uint32_t pa)
{
	uint32_t PD_index = (va) >> PDBITSH;
	page_directory[PD_index].PTBA = (pa >> PTBITSH);
	flush_tlb();
}

/*****************************************************************
 *
 * 	map_video_page
 *  DESCRIPTION:
 *          This function initializes a new Page for a new process
 *  INPUTs: a virtual address and a physical address (passed by reference)
 *  OUTPUT: none
 *  SIDE EFFECTS: Sets up a page directory in memory
 *  SOURCE: http://wiki.osdev.org/Paging#Page_Table
 *          https://courses.engr.illinois.edu/ece391/secure/references/IA32-ref-manual-vol-3.pdf
 *					Helpful pages: 3-22 to 3-28
 *
****************************************************************/
void map_video_page(uint32_t virtualAddr, uint32_t physicalAddr, uint32_t page)
{
	// init page directory entry
    page_directory[virtualAddr/_4MB].PTBA = (((uint32_t)video_page_table)>>12);
    page_directory[virtualAddr/_4MB].present = 1;
    page_directory[virtualAddr/_4MB].read_write = 1;
    page_directory[virtualAddr/_4MB].user_supervisor = 1;

	video_page_table[page].present = 1;
    video_page_table[page].read_write = 1;
    video_page_table[page].user_supervisor = 1;
    video_page_table[page].PBA = physicalAddr>>12;

    flush_tlb();
}

/*****************************************************************
 *
 * 	flush_tlb
 *  DESCRIPTION:
 *          This function utilizes assembly linkage to flush the tlb
 *  INPUTs: none
 *  OUTPUT: none
 *  SIDE EFFECTS: write 0 to the page directory register CR3
 *  SOURCE: http://wiki.osdev.org/TLB
 *
****************************************************************/

void flush_tlb()
{
	asm volatile (
		"movl %0, %%eax;"
		"movl %%eax, %%cr3;"
		:
		: "r"(page_directory)
		: "eax"			// clobbered registers
		);
}
