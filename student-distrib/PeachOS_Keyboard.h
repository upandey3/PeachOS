/* PeachOS_Keyboard.h - Defines for useful keybaord library functions
 * vim:ts=4 noexpandtab
 */

#ifndef KEYBOARD_H
#define KEYBOARD_H

#include "lib.h"
#include "x86_desc.h"
#include "types.h"
#include "i8259.h"

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


/* The following array is taken from
 *    http://www.osdever.net/bkerndev/Docs/keyboard.htm;
 *    https://www.daniweb.com/programming/software-development/code/216732/reading-scan-codes-from-the-keyboard
 *    https://www.win.tue.nl/~aeb/linux/kbd/scancodes-1.html
*/
static uint8_t keyboard_map[TOTAL_CHAR + 1] =
{
    /* -- NORMAL KEYS -- NO CAPS -- NO SHIFT -- */
    '\0', '\0', '1', '2', '3', '4', '5', '6', '7', '8', '9', '0',  '-', '=', '\0',
    '\0', 'q',  'w', 'e', 'r', 't', 'y', 'u', 'i', 'o', 'p', '[',  ']', '\0',
    '\0', 'a',  's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', ';', '\'', '`', '\0',
    '\\', 'z',  'x', 'c', 'v', 'b', 'n', 'm', ',', '.', '/', '\0',
    '*',  '\0', ' ', '\0',

	/* -- SHIFT -- NO CAPS LOCK -- OFFSET: 59 -- */
	'\0', '\0', '!', '@', '#', '$', '%', '^', '&', '*', '(', ')', '_', '+', '\0',
    '\0', 'Q', 'W', 'E', 'R', 'T', 'Y', 'U', 'I', 'O', 'P', '{', '}', '\0',
    '\0', 'A', 'S', 'D', 'F', 'G', 'H', 'J', 'K', 'L' , ':', '"', '~', '\0',
    '|', 'Z', 'X', 'C', 'V', 'B', 'N', 'M', '<', '>', '?', '\0',
    '*', '\0', ' ', '\0',

     /* -- SHIFT -- CAPS  -- OFFSET: 118 -- */
 	'\0', '\0', '!', '@', '#', '$', '%', '^', '&', '*', '(', ')', '_', '+', '\0',
    '\0', 'q', 'w', 'e', 'r', 't', 'y', 'u', 'i', 'o', 'p', '{', '}', '\0',
    '\0', 'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l' , ':', '"', '~', '\0',
    '\\', 'z', 'x', 'c', 'v', 'b', 'n', 'm', '<', '>', '?', '\0',
    '*', '\0', ' ', '\0',

     /* -- CAPS -- NO SHIFT -- OFFSET: 177 -- */
	'\0', '\0', '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '-', '=', '\0',
    '\0', 'Q', 'W', 'E', 'R', 'T', 'Y', 'U', 'I', 'O', 'P', '[', ']', '\0',
    '\0', 'A', 'S', 'D', 'F', 'G', 'H', 'J', 'K', 'L' , ';', '\'', '`', '\0',
    '\\', 'Z', 'X', 'C', 'V', 'B', 'N', 'M', ',', '.', '/', '\0',
    '*', '\0', ' ', '\0'
};


#endif
