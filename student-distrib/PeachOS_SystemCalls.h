#ifndef _SYSTEMCALLS_H
#define _SYSTEMCALLS_H

#include "PeachOS_Terminal.h"
#include "PeachOS_RTC.h"
#include "PeachOS_FileSys.h"
#include "lib.c"

#define MAX_OPEN_FILES 8
#define MAX_FILENAME_SIZE 32

/* File Descriptor Struct */

typedef struct {
    uint32_t *file_jumptable;
    int32_t inode;
    int32_t file_position;
    int32_t flags;
} file_descriptor_t;

/* Process Control Block Struct */

typedef struct {
    file_descriptor_t array[MAX_OPEN_FILES];
    uint8_t filenames[MAX_OPEN_FILES][MAX_FILENAME_SIZE];
    uint8_t process_id;
    uint32_t stack_pointer;
    uint32_t base_pointer;
    uint8_t parent_process_id;
    uint32_t parent_stack_pointer;
    uint32_t parent_base_pointer;
    uint8_t state;
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

int32_t dummy_function(void);

#endif
