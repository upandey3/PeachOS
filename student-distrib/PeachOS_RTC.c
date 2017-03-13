#include "PeachOS_RTC.h"
#include "lib.h"
#include "i8259.h"

void
rtc_init()
{
  unsigned char prev_value;
  outb(INDEX_REGISTER_B, RTC_PORT);
  prev_value = inb(CMOS_PORT);
  outb(INDEX_REGISTER_B, RTC_PORT);
  outb(0x40 | prev_value, CMOS_PORT);

  enable_irq(RTC_IRQ);
}

void
rtc_input_handler(void)
{
  test_interrupts();

  outb(0x0C, RTC_PORT);
  inb(CMOS_PORT);

  send_eoi(RTC_IRQ);
}
