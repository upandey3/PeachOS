/* PeachOS_Keyboard.h - Defines for useful keybaord library functions
 * vim:ts=4 noexpandtab
 */
#ifndef _INTERRUPT_H
#define _INTERRUPT_H

/* Keyboard Handler, going to be used in PeachOS_IDT */
extern keyboard_handler();

/* RTC Handle, going to be used in PeachOS_IDT */
extern void rtc_handler();

#endif
