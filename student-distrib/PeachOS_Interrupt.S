/* # PeachOS_Interrupt.S - Used for Interrupt Handling
# vim:ts=4 noexpandtab */
#define ASM     1 /* BORROWED FROM x86_DESC.S */
#include "x86_desc.h"
.global sysCall_handler
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
.GLOBL interrupt_name             ;\
interrupt_name:                   ;\
  pushal                          ;\
  pushfl                          ;\
  call call_interrupt_handler     ;\
  popfl                           ;\
  popal                           ;\
  iret                            ;\
/* RTC and Keyboard input hanndlers called now */
ALL_INTERRUPT_HANDLER(keyboard_handler, keyboard_input_handler);
ALL_INTERRUPT_HANDLER(rtc_handler, rtc_input_handler);
ALL_INTERRUPT_HANDLER(pit_handler, pit_input_handler);

/*
 *  SYSTEM_CALL_HANDLER
 *  DESCRIPTION:
 *          Assembly wrapper for System Call Handler
 *
 *  INPUT: none
 *
 *  OUTPUT: none
 *  SIDE_EFFECT: This fuction gets called after IDT recognizes the vector passed to it
                  is 0x80; it checks eax for validity and accordingly jumps to the
                  appropriate system call number in the jump table calling each
                  system call function on its own
 *
 *  SOURCES: https://blog.packagecloud.io/eng/2016/04/05/the-definitive-guide-to-linux-system-calls/#kernel-side-int-0x80-entry-point
 *           https://en.wikibooks.org/wiki/X86_Assembly/Interfacing_with_Linux
 *
*/
sysCall_handler:
  /* mask all interrupts */
  cli
  /* Save all registers and flags, except for SS, ESP & CS
      as they are already saved for us by the processor */
  pushl %fs
  pushl %es
  pushl %ds
  pushal
  pushfl
  /* Push arguments in resgiters as per Appendix B, arguments
      live in edx, ecx and ebx but all registers are saved
      in order to stop leaking information to other user
      programs */
  pushl %ebp
  pushl %edi
  pushl %esi
  pushl %edx                  /* 3rd argument */
  pushl %ecx                  /* 2nd argument */
  pushl %ebx                  /* 1st argument */

  movl $0x18, %edx
  movw %dx, %ds
  /* Check to see whether value in eax is valid i.e. whether
      the call number placed in eax by 'int $0x80' is valid
      or not */
  subl $1, %eax
  cmpl $10, %eax
  jae invalid_call             /* if value in eax is greater than 10 */
  /* if call number is valid then unmask interrupts and
      call the corresponding C function from the jump table */
  sti
  call   *sysCall_jumpTable(,%eax,4)
  /* mask all interrupts again and begin restoring registers */
  cli
  /* restore all 6 registers (arguments), assuming they each hold 4 bytes */
restore_all:
  addl $24, %esp
  movl %eax, restore_eax
  /* restore all flags and registers in the correct order */
  popfl
  popal
  popl %ds
  popl %es
  popl %fs
  /* unmask all interrupts and call iret */
  movl restore_eax, %eax
  sti
  iret
invalid_call:
  movl $-1, %eax
  jmp  restore_all
 restore_eax:
      .long 8
sysCall_jumpTable:
  .long SYS_HALT, SYS_EXECUTE, SYS_READ, SYS_WRITE, SYS_OPEN, SYS_CLOSE, SYS_GETARGS, SYS_VIDMAP, SYS_SET_HANDLER, SYS_SIGRETURN
