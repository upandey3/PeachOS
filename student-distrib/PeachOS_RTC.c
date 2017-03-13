#include "PeachOS_RTC.h"
#include "lib.h"
#include "i8259.h"

/*
* void rtc_init();
*   Inputs: none
*   Return Value: none
*	Function: initializes the RTC
* OSDev referenced here: http://wiki.osdev.org/RTC
*/

void
rtc_init()
{
  unsigned char prev_value;                                                     // temporary variable created to store old value
  outb(INDEX_REGISTER_B, RTC_PORT);                                             // select register B and disable NMI
  prev_value = inb(CMOS_PORT);                                                  // read the current value of register B and store
  outb(INDEX_REGISTER_B, RTC_PORT);                                             // reset the index
  outb(0x40 | prev_value, CMOS_PORT);                                           // write the previous value ORed with 0x40, this turns on bit 6 of register B

  enable_irq(RTC_IRQ);                                                          // enable the RTC irq on the PIC
}

/*
* void rtc_input_handler(void);
*   Inputs: void
*   Return Value: none
*	Function: calls the rtc interrupt handler 'test_interrupts()' and
* then selects register C, throws away contents in order to allow
* another interrupt from the RTC
* OSDev referenced here: http://wiki.osdev.org/RTC
*/


void
rtc_input_handler(void)
{
  test_interrupts();                                                            // call test_interrupts() from lib.c

  outb(0x0C, RTC_PORT);                                                         // selects register C
  inb(CMOS_PORT);                                                               // throws away contents

  send_eoi(RTC_IRQ);                                                            // send end of interrupt to PIC
}
