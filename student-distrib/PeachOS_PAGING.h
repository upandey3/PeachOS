#ifndef _PAGING_H
#define _PAGING_H

#include "types.h"

#define ONE_K			  1024
#define FOUR_K			  4096

#define PD_MASK 0xFFC00000
#define OFFSET  0xFFFFF000
#define PDBITSH 22
#define PTBITSH 12

//struct for page directory entries
typedef struct page_directory {
		struct {
			uint32_t present : 1;
			uint32_t read_write : 1;
			uint32_t user_supervisor : 1;
			uint32_t write_through : 1;
			uint32_t cache_disabled : 1;
			uint32_t accessed : 1;
			uint32_t reserved : 1;
			uint32_t page_size : 1;
			uint32_t global_page : 1;
			uint32_t available : 3;
			uint32_t PTBA : 20;	//page table base address
		} __attribute__((packed));
} page_directory_t;

//struct for page table entries
typedef struct page_table {
		struct {
			uint32_t present : 1;
			uint32_t read_write : 1;
			uint32_t user_supervisor : 1;
			uint32_t write_through : 1;
			uint32_t cache_disabled : 1;
			uint32_t accessed : 1;
			uint32_t reserved : 1;
			uint32_t page_size : 1;
			uint32_t global_page : 1;
			uint32_t available : 3;
			uint32_t PBA : 20;	//page table base address
		} __attribute__((packed));
} page_table_t;

extern page_directory_t page_directory[ONE_K] __attribute__((aligned(FOUR_K)));
extern page_table_t page_table[ONE_K] __attribute__((aligned(FOUR_K)));

/* Single function to create page directory and enable paging */
void paging_init (void);

/* Function to create page directory (0-4GB) and first page table (0-4MB)*/
void pageDirectory_init (void);

/* Assembly function to enable paging bits in cr0, cr3, and cr4 */
void enablePaging (void);

/* Initializes a new page directory index for a new process */
void init_page (uint32_t va, uint32_t pa);

/* This function uses assembly linkage to flush the tlb */
void flush_tlb();

#endif /* _PAGING_H */
