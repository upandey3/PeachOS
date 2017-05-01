/* PeachOS_Keyboard.h - Defines for useful keybaord library functions
 * vim:ts=4 noexpandtab
 */
#ifndef _TERMINAL_H
#define _TERMINAL_H

#include "lib.h"
#include "i8259.h"
#include "types.h"
#include "PeachOS_Keyboard.h"
#include "PeachOS_IDT.h"
#include "PeachOS_Interrupt.h"
#include "PeachOS_SystemCall_Test.h"
#include "PeachOS_SystemCalls.h"
#include "PeachOS_PAGING.h"



/*
 * Taken from tuxctl-ld.c
*/
#define TERMINAL_BUFSIZE 128
#define MAX_TERMINAL 3

#define TERMINAL_SIZE 2*NUM_ROWS*NUM_COLS

typedef struct terminal_data
{
    uint8_t terminal_buf[TERMINAL_BUFSIZE]; // terminal buffer to read and write away from keyboard_buffer
    uint8_t terminal_index; //which terminal it is

    uint32_t terminal_x_pos; // keeping track of screen position
    uint32_t terminal_y_pos;

    int32_t displayed; // flag to specify whether terminal is being displayed
    int16_t pid; // process id running, if -1 no process running
    char* terminal_video_mem; // use this to put things on the screen
} terminal_data_t;

terminal_data_t terminal[MAX_TERMINAL]; // creating terminal 1, 2, 3
extern uint8_t terminal_colors[MAX_TERMINAL]; // contains colors for terminal 1, 2, 3
extern uint8_t term_init_flag;
extern void terminal_init(void);
void terminal_launch(uint8_t terminal_num);
void terminal_switch(uint8_t terminal_num);
void terminal_save_current_terminal(uint8_t terminal_num);
void terminal_restore_new_terminal(uint8_t terminal_num);

int32_t terminal_open(const uint8_t* filename); // got this from http://freesoftwaremagazine.com/articles/drivers_linux/
int32_t terminal_close(int32_t fd); // same ^

int32_t terminal_read(int32_t fd, void* buf, int32_t nbytes); // in ece391hello.c they send in empty buffer, ece391syscall.h
int32_t terminal_write(int32_t fd, const void* buf, int32_t nbytes); // in ece391hello.c they send in empty buffer, ece391syscall.h

int32_t terminal_test();


#endif
