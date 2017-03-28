#ifndef _FILESYS_H
#define _FILESYS_H

#include "types.h"
#include "lib.h"
#include "PeachOS_Terminal.h"

#define BOOT_BLOCK_SIZE 64
#define DENTRY_BLOCK_SIZE 64
#define FILE_NAME_SIZE 32
#define FILE_TYPE_SIZE 4
#define ABSOLUTE_BLOCK_SIZE 4096
#define NUM_DENTRIES_SIZE 4
#define LENGTH_IN_BYTES_SIZE 4
#define MAX_BYTE_WIDTH 6 // Formatting. Determined by comparing printf output with terminal size

extern uint32_t file_sys_addr;
//extern uint32_t file_num_t;

//struct for page directory entries
typedef struct dentry {
	unsigned char file_name[FILE_NAME_SIZE+1];
	uint32_t file_type;
	uint32_t inode_number;
} dentry_t;

int32_t read_dentry_by_name (const uint8_t* fname, dentry_t* dentry);
int32_t read_dentry_by_index (uint32_t index, dentry_t* dentry);
int32_t read_data (uint32_t inode, uint32_t offset, uint8_t* buf, uint32_t length);

int32_t print_directory(void);
int32_t print_file_by_name(const uint8_t* fname);
int32_t print_file_by_index(uint32_t file_num);

#endif /* _FILESYS_H */
