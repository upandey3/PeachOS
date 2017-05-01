
#ifndef _SYSTEMCALLS_H
#define _SYSTEMCALLS_H

#include "types.h"
#include "x86_desc.h"
#include "PeachOS_Scheduling.h"

#define MAX_OPEN_FILES 8
#define MAX_FILENAME_SIZE 32

#define MAX_PROCESSES 6

#define LOWER_13_BITS_MASK 0xFFFFE000
#define HIGHER_10_BITS_MASK 0xFFC00000

#define NOT_AVAILABLE 0
#define AVAILABLE 1

#define FIRST_FD 2
#define LAST_FD 7

#define OFFSET0 0

#define FILE 2
#define DIR 1
#define RTC 0

#define OPEN 0
#define READ 1
#define WRITE 2
#define CLOSE 3

#define ARGSIZE 100
#define NUM_ZERO 0
#define NUM_FOUR 4

#define SHELL_NAME_SIZE

#define ASCII_DEL 0x7f
#define ASCII_E 0x45
#define ASCII_L 0x4C
#define ASCII_F 0x46

/* Function Pointers for File Descriptor jump_table */
/* Struct: jump_table_ops
 *    fd_read: Function Pointer that points to the corresponding read function
 *	  fd_write: Function Pointer that points to the corresponding write function
 * 	  fd_open: Function Pointer that points to the corresponding open function
 *	  fd_close: Function Pointer that points to the corresponding close function
 *
 * SOURCE : http://stackoverflow.com/questions/9932212/jump-table-examples-in-c
 *          http://www.sanfoundry.com/c-tutorials-jump-tables/
 *          http://www.microchip.com/forums/m798465.aspx
*/
typedef struct {
    int32_t (*fd_read)(int32_t fd, void* buffer, int32_t nbytes);
    int32_t (*fd_write)(int32_t fd, const void* buffer, int32_t nbytes);
    int32_t (*fd_open)(const uint8_t* fname);
    int32_t (*fd_close)(int32_t fd);
} jump_table_ops;

/* File Descriptor Struct */
typedef struct {
    jump_table_ops file_jumptable;
    int32_t inode;
    int32_t file_position;
    int32_t flags;
} file_descriptor_t;

/* Process Control Block Struct */
typedef struct {
    file_descriptor_t open_files[MAX_OPEN_FILES];
    uint8_t filenames[MAX_OPEN_FILES][MAX_FILENAME_SIZE];
    int8_t process_id;
    int8_t parent_process_id;
    uint8_t args[ARGSIZE];
    uint32_t stack_pointer;
    uint32_t base_pointer;
    uint32_t parent_stack_pointer;
    uint32_t parent_base_pointer;
    uint32_t parent_tss;
    uint32_t parent_pcb;
    uint8_t state;
    uint8_t timeslice;
} pcb_t;

/* FUNCTION DECLARATIONS */
/*** SYSTEM CALLS ***/
int32_t SYS_HALT(uint8_t status);
int32_t SYS_EXECUTE(const uint8_t* command);
int32_t SYS_READ(int32_t fd, void* buf, int32_t nbytes);
int32_t SYS_WRITE(int32_t fd, const void* buf, int32_t nbytes);
int32_t SYS_OPEN(const uint8_t* filename);
int32_t SYS_CLOSE(int32_t fd);
int32_t SYS_GETARGS(uint8_t* buf, int32_t nbytes);
int32_t SYS_VIDMAP(uint8_t** screen_start);
int32_t SYS_SET_HANDLER(int32_t signum, void* handler_address);
int32_t SYS_SIGRETURN(void);

/*** HELPER FUNCTIONS ***/
uint32_t parse_command(const uint8_t* command, uint8_t* file_name, uint8_t* arg_buffer);
uint32_t check_executable(uint8_t* file_name, uint32_t* dir_ptr);
pcb_t *get_curr_pcb();
extern int32_t get_available_process_num();
uint32_t set_available_process_num();
int32_t dummy_function();
pcb_t * pcb_fork(uint32_t process_num, pcb_t * parent_pcb);
pcb_t * get_pcb_by_process(uint8_t process_num);
extern uint8_t available_processes[MAX_PROCESSES];
#endif
