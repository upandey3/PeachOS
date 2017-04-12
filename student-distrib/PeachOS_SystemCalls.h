#ifndef _SYSTEMCALLS_H
#define _SYSTEMCALLS_H

#include "PeachOS_Terminal.h"
#include "PeachOS_RTC.h"
#include "PeachOS_FileSys.h"
#include "lib.c"

#define MAX_OPEN_FILES 8
#define MAX_FILENAME_SIZE 32
#define LOWER_13_BITS_MASK 0xFFFFE000
#define NOT_AVAILABLE 0
#define AVAILABLE 1
#define FIRST_FD 2
#define LAST_FD 7
#define OFFSET0 0x0
#define FILE 2
#define DIR 1
#define RTC 0
#define OPEN 0
#define READ 1
#define WRITE 2
#define CLOSE 3
#define argsize 100

/* File Descriptor Struct */

typedef struct {
    uint32_t *file_jumptable;
    int32_t inode;
    int32_t file_position;
    int32_t flags;
} file_descriptor_t;

/* Process Control Block Struct */


typedef struct {
    file_descriptor_t open_files[MAX_OPEN_FILES];
    uint8_t filenames[MAX_OPEN_FILES][MAX_FILENAME_SIZE];
    uint8_t process_id;
    uint8_t args[argsize];
    uint32_t stack_pointer;
    uint32_t base_pointer;
    uint8_t parent_process_id;
    uint32_t parent_stack_pointer;
    uint32_t parent_base_pointer;
    uint8_t state;
    unsigned int timeslice;
} pcb_t;

/* FUNCTION DECLARATIONS */

/*** SYSTEM CALLS ***/

int32_t halt(uint8_t status);
int32_t execute(const uint8_t* command);
int32_t read(int32_t fd, void* buf, int32_t nbytes);
int32_t write(int32_t fd, const void* buf, int32_t nbytes);
int32_t open(const uint8_t* filename);
int32_t close(int32_t fd);
int32_t getargs(uint8_t* buf, int32_t nbytes);
int32_t vidmap(uint8_t** screen_start);
int32_t set_handler(int32_t signum, void* handler_address);
int32_t sigreturn(void);

/*** HELPER FUNCTIONS ***/
pcb_t * get_curr_pcb(){

#endif
