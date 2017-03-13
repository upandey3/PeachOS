# PeachOS_Interrupt.S - Used for Interrupt Handling
# vim:ts=4 noexpandtab

#define ASM     1 /* BORROWED FROM x86_DESC.S */
#include "x86_desc.h"

#define ALL_INTERRUPT_HANDLER(interrupt_name, call_interrupt_handler)   \
.GLOBL interrupt_name               ;\
interrupt_name:                     ;\
    pushal                          ;\
    pushfl                          ;\
    call call_interrupt_handler     ;\
    popfl                           ;\
    popal                           ;\
    iret                            ;\

ALL_INTERRUPT_HANDLER(keyboard_handler, keyboard_input_handler);
ALL_INTERRUPT_HANDLER(rtc_handler, rtc_input_handler);
