#ifndef _PIT_H
#define _PIT_H

#include "types.h"
#include "lib.h"
#include "i8259.h"
#include "PeachOS_Scheduling.h"

#define PIT_IRQ 0

#define PIT_CMD_PORT 0x43

/* Channel 0 IO Port is used as it is connected to the PIC at IRQ = 0 */

#define PIT_CH0_IOPORT 0x40

#define PIT_DEFAULT_FREQ 1193182

/* 8 bit setting required to be written to the PIT_CMD_PORT
*  Bits 6 and 7 - 0 0 (Channel 0 selected as channel 0 writes to IRQ0)
*  Bits 4 and 5 - 1 1 (Both lo and hi bytes should be accessed)
*  Bits 1, 2 and 3 - 0 1 1 (Mode 3 selected as this utilizes the frequency divider)
*  Bit 0 - 0
*  Source: http://wiki.osdev.org/Programmable_Interval_Timer */

#define PIT_SETTING 0x36  // 0 0 1 1 0 1 1 0 = 0x36

#define LOW_MASK 0xFF
#define BIT_SHIFT 8

/* function declaration for PIT initialization */
void pit_init();

/* function declaration for helper function that sets PIT frequency */
void timer_set(uint16_t freq);

/* function declaration for PIT interrupt handler */
void pit_input_handler();

#endif
