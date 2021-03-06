/* PeachOS_Keyboard.h - Defines for useful keybaord library functions
 * vim:ts=4 noexpandtab
 */
#ifndef _INTERRUPT_H
#define _INTERRUPT_H
/*
 * keyboard_init
 *  DESCRIPTION:
 *          Keyboard Handler, going to be used in PeachOS_IDT
 *
 *  INPUT: none
 *
 *  OUTPUT: none
 *  SIDE_EFFECT: This fuction is mapped to the IDT in PeachOS_IDT.c, everytime
 *                  an interrupt is generated by the keyboard, this function
 *                  will be called, and from here we call our keyboard_input_handler
 *  SOURCE: http://arjunsreedharan.org/post/99370248137/kernel-201-lets-write-a-kernel-with-keyboard
 *          Refer Section: Keyboard interrupt handling function
*/
extern void keyboard_handler();
/*
 * keyboard_init
 *  DESCRIPTION:
 *          RTC Handler, going to be used in PeachOS_IDT
 *
 *  INPUT: none
 *
 *  OUTPUT: none
 *  SIDE_EFFECT: This fuction is mapped to the IDT in PeachOS_IDT.c, everytime
 *                  an interrupt is generated by the RTC, this function
 *                  will be called, and from here we call our rtc_input_handler
 *  SOURCE: none, but similiar to keybaord hadling
*/
extern void rtc_handler();
extern void sysCall_handler();
extern void pit_handler();

#endif
