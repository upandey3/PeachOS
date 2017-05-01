/* PeachOS_Keyboard.h - Defines for useful keybaord library functions
 * vim:ts=4 noexpandtab
 */
#ifndef _KEYBOARD_H
#define _KEYBOARD_H

#include "lib.h"
#include "x86_desc.h"
#include "types.h"
#include "i8259.h"
#include "PeachOS_Interrupt.h"
#include "PeachOS_RTC.h"
#include "PeachOS_Terminal.h"
#include "PeachOS_FileSys.h"
#include "PeachOS_SystemCalls.h"

#define LIMIT 128

/* BUFFER TO HOLD VALUES */
uint8_t* keyboard_buffer;
uint32_t keyboard_index; //index for the array

volatile uint32_t janky_spinlock_flag; // janky_spinlock for waiting to get the kebaord input
volatile uint32_t buffer_limit_flag; // overflow flag
uint32_t terminal_flag_keyboard; // terminal flag, set if we are in terminal mode

/* -- MASTER PIC- IRQ1 IS FOR KEYBOARD --  */
#define KEYBOARD_IRQ	   1

/*
 *  KEYBOARD_DATA_REGISTER will be used to get keyboard input from
 *  KEYBOARD_CONTROL_REGISTER will be useful in setting LEDS
*/
#define KEYBOARD_DATA_REGISTER          0x60
#define KEYBOARD_CONTROL_REGISTER       0x64

/* Total keys in keybaord we use */
#define CHAR_COUNT         59
#define TOTAL_CHAR         CHAR_COUNT * 4

/* OFFSETS FOR DIFFERENT KEY LAYOUTS*/
#define NORMAL_KEYS        0
#define SHIFT_NO_CAPS      59
#define SHIFT_CAPS         118
#define NO_SHIFT_CAPS      177

/* SPECIAL CHARACTERS DEFINE BY ME */
#define ESC                0x01
#define BACKSPACE          0x0E
#define TAB                0x0F
#define ENTER_KEY          0x1C
#define L_CTRL_PRESSED     0x1D
#define L_CTRL_UNPRESSED   0x9D
#define L_SHIFT_PRESSED    0x2A
#define L_SHIFT_UNPRESSED  0xAA
#define R_SHIFT_PRESSED    0x36
#define R_SHIFT_UNPRESSED  0xB6
#define ALT_PRESSED        0x38
#define ALT_UNPRESSED      0xB8
#define CAPS_LOCK          0x3A
#define F1_PRESSED         0x3B
#define F2_PRESSED         0x3C
#define F3_PRESSED         0x3D

/* DEFINE PRESSED, UNPRESSED... Will be used later to set CTRL, ALT, SHIFT keys */
#define PRESSED 1
#define UNPRESSED 0


/* Intialize the keyboard interrupt */
extern void keyboard_init();

/* Used to handle keyboard interrupts */
extern void keyboard_input_handler();

/* Once key pressed detected, we figure out which key it is */
extern void keyboard_key_pressed(uint8_t keyboard_value);

/* Special case for enter key */
extern void keyboard_enter_key_pressed();

/* Special case for backspace key */
extern void keyboard_backspace_key_pressed();

/* Clear out the buffer */
void empty_buffer(uint8_t* keyboard_buffer);


#endif
