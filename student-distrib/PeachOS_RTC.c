#include "PeachOS_RTC.h"
#include "lib.h"
#include "i8259.h"
#include "PeachOS_Terminal.h"

volatile int rtc_flag = 0;
int frequency_var;
int rtc_test_flag;

/*
*   void rtc_init();
*
*   Inputs: none
*   Return Value: none
*	  Function: initializes the RTC
*   OSDev referenced here: http://wiki.osdev.org/RTC
*/

/* check first outb statement below */

void
rtc_init()
{
  unsigned char prev_value;                                                     // temporary variable created to store old value
  outb(INDEX_REGISTER_A, RTC_PORT);                                             // select register B and disable NMI
  prev_value = inb(CMOS_PORT);                                                  // read the current value of register B and store
  outb(INDEX_REGISTER_B, RTC_PORT);                                             // reset the index
  outb(INITMASK | prev_value, CMOS_PORT);

  frequency_var = 2;
  rtc_test_flag = 0;                                                            // write the previous value ORed with 0x40, this turns on bit 6 of register B

  enable_irq(RTC_IRQ);                                                          // enable the RTC irq on the PIC
}

/*
*   void rtc_input_handler(void);
*
*   Inputs: void
*   Return Value: none
*	  Function: masks all interrupts at first then throws away contents in order to allow
*   another interrupt from the RTC before sending the eoi and then setting the
*   global interrupt flag to 1
*   OSDev referenced here: http://wiki.osdev.org/RTC
*/


void
rtc_input_handler(void)
{
  cli();                                                                        // mask all interrupts

  outb(0x0C, RTC_PORT);                                                         // selects register C
  inb(CMOS_PORT);                                                               // throws away contents
  send_eoi(RTC_IRQ);                                                            // send end of interrupt to PIC
  rtc_flag = 1;                                                                 // clear the interrupt flag

  sti();                                                                        // unmask all interrupts
}

/*
*   int32_t read(int32_t fd, void* buf, int32_t nbytes);
*
*   Inputs: file descriptor, buffer being read and number of bytes
*   Return Value: returns 0 upon success
*	  Function: Reads the contents of the RTC only after an interrupt has occured
*/

int32_t
rtc_read(int32_t fd, void* buf, int32_t nbytes)
{
  while (rtc_flag == 0)                                                         // keep spinning and waiting till
  {                                                                             // the handler has cleared the flag
                                                                                // do nothing inside the while loop
  }

  rtc_flag = 0;                                                                 // set the flag
  return 0;                                                                     // always return 0
}

/*
*   void freq_set(int32_t arg);
*
*   Inputs: frequency to be set
*   Return Value: none
*	  Function: helper function that sets the frequency of the rtc as defined
*   by the user in the function argument
*   OSDev referenced here: http://wiki.osdev.org/RTC
*   RTC datasheet too: https://courses.engr.illinois.edu/ece391/secure/references/ds12887.pdf (Page 19)
*/

void freq_set(int32_t arg)
{
  unsigned char temp;                                                           // declare temporary variables
  unsigned char prev_value;                                                     // for holding previous port values

  outb(INDEX_REGISTER_A, RTC_PORT);                                             // save old value of register A
  prev_value = inb(CMOS_PORT);                                                  // since we are writing to it

  if (arg > MAX_RTC)                                                            // check if frequency value is above 1024
  {                                                                             // if it is, then do nothing and return
    return;
  }

  else                                                                          // else, set temporary variable to hex
  {                                                                             // equivalents defined in the RTC datasheet
    switch(arg)
    {
      case 1024:
          temp = TEN;
          break;
      case 512:
          temp = NINE;
          break;
      case 256:
          temp = EIGHT;
          break;
      case 128:
          temp = SEVEN;
          break;
      case 64:
          temp = SIX;
          break;
      case 32:
          temp = FIVE;
          break;
      case 16:
          temp = FOUR;
          break;
      case 8:
          temp = THREE;
          break;
      case 4:
          temp = TWO;
          break;
      case 2:
          temp = ONE;
          break;
      case 0:
          temp = ZERO;
          break;
      default:
          break;
    }
  }

  outb(INDEX_REGISTER_A, RTC_PORT);                                             // reset index to register A
  outb((prev_value & THEMASK) | temp, CMOS_PORT);                               // AND with mask to extract the lower 4 bits and OR with temp
  return;
}

/*
*   int32_t write(int32_t fd, const void* buf, int32_t nbytes);
*
*   Inputs: file descriptor, buffer to write into and number of bytes written
*   Return Value: returns -1 on failure and nbytes on success
*	  Function: extracts the desired frequency from the buffer
*   and calls the helper function in order to set frequency of the rtc
*/

int32_t
rtc_write(int32_t fd, const void* buf, int32_t nbytes)
{
  if (nbytes != 4)                                                              // check if nbytes is not equal to 4
  {
    return -1;                                                                  // if it is then, return -1
  }

  if (buf == NULL)                                                              // Added post 3.2 as a fix
  {
    return -1;
  }

  int32_t temp;                                                                 // temporary variable declared
  temp = *((int32_t*) buf);                                                     // extract the frequency from the buffer

  if (temp == NULL)                                                             // check if it is NULL
  {
    return -1;                                                                  // if it is then, return -1
  }

  else
  {
    freq_set(temp);                                                             // else, call the helper function
    return nbytes;                                                              // to set frequency and return nbytes
  }
}

/*
*   int32_t open(const uint8_t* filename);
*
*   Inputs: constant pointer to filename
*   Return Value: always returns 0
*	  Function: Opens the rtc and then sets frequency to 2 Hz
*   via the helper function as required by the documentation
*/

int32_t
rtc_open(const uint8_t* filename)
{
  freq_set(2);                                                                  // set frequency to 2 Hz
  rtc_flag = 0;
  return 0;                                                                     // and return 0
}

/*
*   int32_t close(int32_t fd);
*
*   Inputs: file descriptor to be closed
*   Return Value: always returns 0
*	  Function: Closes the rtc but before doing so resets the
*   frequency to 2 Hz
*/

int32_t
rtc_close(int32_t fd)
{
  freq_set(2);                                                                  // reset frequency to 2 Hz
  return 0;                                                                     // and return 0
}

/*
*   RTC test function: void rtc_test(void);
*
*   Inputs: void
*   Return Value: void
*	  Function: Tests the RTC periodic interrupts
*/

void
rtc_test(void)
{
  int temp;
  frequency_var = frequency_var * 2;                                            // keep increasing the frequency by powers of 2 each time function is called

  if (frequency_var > 1024)                                                     // when frequency reaches max value, reset back to 2
  {
    frequency_var = 2;                                                          // as per the test cases gif
  }

  rtc_write(0, &frequency_var, 4);                                              // call RTC write with current frequency
  do {                                                                          // while the flag is cleared, keep printing 1's to terminal
    if (rtc_read(0, &temp, 4) == 0)
    {
      if (x_position() != 79)                                                   // if x_pos reaches end of the screen, go to next line
      {
        terminal_write(1, (char*)"1", 1);
      }
      else
      {
        printf("\n");
      }
    }
  } while(rtc_test_flag);

  clear();                                                                      // clear screen at the end
}
