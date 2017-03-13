# PeachOS_Interrupt.S - Used for Interrupt Handling
# vim:ts=4 noexpandtab

#define ASM     1 /* BORROWED FROM x86_DESC.S */
#include "x86_desc.h"

/*
 * ALL_INTERRUPT_HANDLER
 *  DESCRIPTION:
 *          Handler for interrupts for RTC, KEYBOARD
 *
 *  INPUT: none
 *
 *  OUTPUT: none
 *  SIDE_EFFECT: This fuction gets called after IDT recognizes the interrupt to either RTX or Keybaord
 *                  From here we call the actual interrupt hadlers
 *
 *  SOURCE: http://arjunsreedharan.org/post/99370248137/kernel-201-lets-write-a-kernel-with-keyboard
 *          Refer Section: Keyboard interrupt handling function
*/
#define ALL_INTERRUPT_HANDLER(interrupt_name, call_interrupt_handler)   \
.GLOBL interrupt_name               ;\
interrupt_name:                     ;\
    pushal                          ;\
    pushfl                          ;\
    call call_interrupt_handler     ;\
    popfl                           ;\
    popal                           ;\
    iret                            ;\

/* RTC and Keyboard input hanndlers called now */
ALL_INTERRUPT_HANDLER(keyboard_handler, keyboard_input_handler);
ALL_INTERRUPT_HANDLER(rtc_handler, rtc_input_handler);
