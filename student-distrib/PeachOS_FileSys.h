#ifndef _FILESYS_H
#define _FILESYS_H

#include "types.h"
#include "lib.h"
#include "PeachOS_Terminal.h"
#include "PeachOS_SystemCalls.h"

#define FILE_TYPE_SIZE 4
#define MAX_BYTE_WIDTH 6 // Formatting. Determined by comparing printf output with terminal size

extern uint32_t fs_ctrl_3;

#define FILE_TYPE_SIZE 4
#define MAX_BYTE_WIDTH 6 // Formatting. Determined by comparing printf output with terminal size

extern uint32_t fs_ctrl_3;

#define FILENAMESIZE 32
#define DERESERVED 24
#define BBRESERVED 52
#define DIRENTRIES 63
#define DATABLOCKS 1023
#define FILESTATS 64
#define BLOCK_SIZE 4096

//struct for page directory entries
typedef struct {
  int8_t filename[FILENAMESIZE];
  int32_t filetype;
  int32_t inode;
  uint8_t de_reserved[DERESERVED];
} dentry_t;

typedef struct {
  int32_t num_directories;
  int32_t num_inodes;
  int32_t num_datablocks;
  uint8_t bb_reserved[BBRESERVED];
  dentry_t dirEntries[DIRENTRIES];
} bootblock_t;

typedef struct {
  int32_t filelength;
  int32_t dataBlock[DATABLOCKS];
} inode_t;


int32_t read_dentry_by_name (const uint8_t* fname, dentry_t* dentry);
int32_t read_dentry_by_index(uint32_t index, dentry_t* dentry);
int32_t read_data (uint32_t inode, uint32_t offset, uint8_t* buf, uint32_t length);

int32_t print_directory(void);
int32_t print_file_by_name(const uint8_t* fname);
int32_t print_file_by_index(uint32_t file_num);
void fileSystem_init(uint32_t fsAddr);

/*
 * File operation FUNCTIONS
*/
int32_t open_file(const uint8_t * filename);
int32_t close_file(int32_t fd);
int32_t read_file(int32_t fd, void * buf, int32_t nbytes);
int32_t write_file(int32_t fd, const void * buf, int32_t nbytes);

/*
 * Direcotry operation FUNCTIONS
*/
int32_t open_directory(const uint8_t * dname);
int32_t close_directory(int32_t fd);
int32_t read_directory(int32_t fd, void * buf, int32_t nbytes);
int32_t write_directory(int32_t fd, const void * buf, int32_t nbytes);

extern dentry_t* dirEntries;
extern inode_t* inodes;
extern uint32_t num_directories;
extern uint32_t num_inodes;
extern uint32_t num_datablocks;
extern uint32_t bbAddr;
extern uint32_t dataStart;
extern uint32_t fs_ctrl_3;

#endif /* _FILESYS_H */
