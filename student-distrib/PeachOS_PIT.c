#include "PeachOS_PIT.h"

/*
*   void pit_init();
*
*   Inputs: none
*   Return Value: none
*	  Function: initializes the PIT, by writing the correct
*   setting in bits to the command port
*   Sources: http://wiki.osdev.org/Programmable_Interval_Timer
*           https://littleosbook.github.io/#programmable-interval-timer
*/
void
pit_init()
{
  outb(PIT_SETTING, PIT_CMD_PORT);

  /* 42614 is used here as the divider since it results in a frequency
     of about 28 Hz which translates to about 35.7 ms */

  uint16_t frequency = /*65535;*/ 42614;
  timer_set(frequency);

  enable_irq(PIT_IRQ);

  return;
}

/*
*   void timer_set (uint16_t freq);
*
*   Inputs: frequency to be set
*   Outputs: none
*	  Function: helper function that sets the frequency of the PIT
*   using a divider and default frequency of the PIT
*   Sources: http://wiki.osdev.org/Programmable_Interval_Timer
*           https://littleosbook.github.io/#programmable-interval-timer
*/
void
timer_set (uint16_t freq)
{
  // uint16_t frequency = (PIT_DEFAULT_FREQ)/(freq);

  uint8_t low_bits = (freq & LOW_MASK);
  uint8_t high_bits = ((freq >> BIT_SHIFT) & (LOW_MASK));

  outb(low_bits, PIT_CH0_IOPORT);
  outb(high_bits, PIT_CH0_IOPORT);

  return;
}

/*
*   void pit_input_handler();
*
*   Inputs: void
*   Return Value: none
*	  Function: called everytime an interrupt is generated
*   masks all interrupts at first then acknowledges interrupts
*   by sending an eoi to the PIC, then makes a call to schedule a process
*   before unmasking interrupts again
*   Source: http://www.brokenthorn.com/Resources/OSDevPit.html
*/
// int is = 150;
void
pit_input_handler()
{

    send_eoi(PIT_IRQ);

    cli();

    //Call the scheduler
    // run_scheduler();

    sti();
}
