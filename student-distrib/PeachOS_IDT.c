#include "lib.h"
#include "PeachOS_IDT.h"
#include "x86_desc.h"
#include "PeachOS_RTC.h"

#include "PeachOS_Keyboard.h"
#include "PeachOS_Interrupt.h"

void initialize_idt();

#define INTERRUPT_HANDLER(interrupt_name, string) \
void interrupt_name () { \
  printf("%s\n", #string); \
  while(1); \
}

#define RTC_HANDLER()

INTERRUPT_HANDLER (DIVIDE_BY_ZERO, "DIVIDE BY ZERO");
INTERRUPT_HANDLER (DEBUG, "DEBUG");
INTERRUPT_HANDLER (NON_MASKABLE_INTERRUPT, "NON MASKABLE INTERRUPT");
INTERRUPT_HANDLER (BREAKPOINT, "BREAKPOINT");
INTERRUPT_HANDLER (OVERFLOW, "OVERFLOW");
INTERRUPT_HANDLER (BOUND_RANGE_EXCEEDED, "BOUND RANE EXCEEDED");
INTERRUPT_HANDLER (INVALID_OPCODE, "INVALID OPCODE");
INTERRUPT_HANDLER (DEVICE_NOT_AVAILABLE, "DEVICE NOT AVAILABLE");
INTERRUPT_HANDLER (DOUBLE_FAULT, "DOUBLE FAULT");
INTERRUPT_HANDLER (COPROCESSOR_SEGMENT_OVERRUN, "COPROCESSOR SEGMENT OVERRUN");
INTERRUPT_HANDLER (INVALID_TSS, "INVALID TSS");
INTERRUPT_HANDLER (SEGMENT_NOT_PRESENT, "SEGMENT NOT PRESENT");
INTERRUPT_HANDLER (STACK_SEGMENT_FAULT, "STACK SEGMENT FAULT");
INTERRUPT_HANDLER (GENERAL_PROTECTION_FAULT, "GENERAL PROTECTION FAULT");
INTERRUPT_HANDLER (PAGE_FAULT, "PAGE FAULT");
INTERRUPT_HANDLER (FLOATING_POINT_ERROR, "FLOATING POINT ERROR");
INTERRUPT_HANDLER (ALIGNMENT_CHECK, "ALIGNMENT CHECK");
INTERRUPT_HANDLER (MACHINE_CHECK, "MACHINE CHECK");
INTERRUPT_HANDLER (SIMD_FLOATING_POINT_EXCEPTION, "SIMD FLOATING POINT EXCEPTION");
INTERRUPT_HANDLER (UNDEFINED_INTERRUPT, "Interrupt not defined by our OS!");
INTERRUPT_HANDLER (SYSTEM_CALL, "System Call Generated!");

/* Set to 0 1 1 0 0 (call gate) simply allows privilege transfer from lower to higher */

/* Set to 0 1 1 1 0 (32 bit interrupt gate) handles hardware, user-defined interrupts and system calls
   so that these interrupts/system calls are automatically disabled by clearing IF */

void initialize_idt () {

  int i;

  for (i = 0; i < NUM_VEC; i++)
  {
    idt[i].seg_selector = KERNEL_CS;
    idt[i].reserved4 = 0x0;
    idt[i].reserved3 = 0x0;
    idt[i].reserved2 = 0x1;
    idt[i].reserved1 = 0x1;
    idt[i].size = 0x1;
    idt[i].reserved0 = 0x0;
    idt[i].dpl = 0x0;
    idt[i].present = 0x1;

    if (i > PIC_INT)
    {
      idt[i].reserved3 = 0x1;
      SET_IDT_ENTRY(idt[i], UNDEFINED_INTERRUPT);
    }

    if (i == SYS_CAL)
    {
      idt[i].reserved3 = 0x1;
      idt[i].dpl = 0x3;
    }
  }

  SET_IDT_ENTRY (idt[0], DIVIDE_BY_ZERO);
  SET_IDT_ENTRY (idt[1], DEBUG);
  SET_IDT_ENTRY (idt[2], NON_MASKABLE_INTERRUPT);
  SET_IDT_ENTRY (idt[3], BREAKPOINT);
  SET_IDT_ENTRY (idt[4], OVERFLOW);
  SET_IDT_ENTRY (idt[5], BOUND_RANGE_EXCEEDED);
  SET_IDT_ENTRY (idt[6], INVALID_OPCODE);
  SET_IDT_ENTRY (idt[7], DEVICE_NOT_AVAILABLE);
  SET_IDT_ENTRY (idt[8], DOUBLE_FAULT);
  SET_IDT_ENTRY (idt[9], COPROCESSOR_SEGMENT_OVERRUN);
  SET_IDT_ENTRY (idt[10], INVALID_TSS);
  SET_IDT_ENTRY (idt[11], SEGMENT_NOT_PRESENT);
  SET_IDT_ENTRY (idt[12], STACK_SEGMENT_FAULT);
  SET_IDT_ENTRY (idt[13], GENERAL_PROTECTION_FAULT);
  SET_IDT_ENTRY (idt[14], PAGE_FAULT);
  SET_IDT_ENTRY (idt[15], FLOATING_POINT_ERROR);
  SET_IDT_ENTRY (idt[16], ALIGNMENT_CHECK);
  SET_IDT_ENTRY (idt[17], MACHINE_CHECK);
  SET_IDT_ENTRY (idt[18], SIMD_FLOATING_POINT_EXCEPTION);
  SET_IDT_ENTRY (idt[SYS_CAL], SYSTEM_CALL);
  SET_IDT_ENTRY (idt[RTC_INT], rtc_handler);
  SET_IDT_ENTRY (idt[KBD_INT], keyboard_handler);

  lidt(idt_desc_ptr);

  return;
}
