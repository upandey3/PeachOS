/* PeachOS_Interrupt.h - Defines for useful keybaord library functions
 * vim:ts=4 noexpandtab
 */
#ifndef _INTERRUPT_H
#define _INTERRUPT_H

extern void keyboard_handler();

extern void rtc_handler();

extern void pit_handler();

extern void sysCall_handler();

#endif
