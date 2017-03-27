#ifndef _RTC_H
#define _RTC_H

#include "types.h"

/* function declaration for rtc initialization */
extern void rtc_init();

/* function declaration for rtc interrupt handler */
extern void rtc_input_handler(void);

/* function declaration for rtc system call: read */
int32_t rtc_read(int32_t fd, void* buf, int32_t nbytes);

/* function declaration for rtc system call: write */
int32_t rtc_write(int32_t fd, const void* buf, int32_t nbytes);

/* function declaration for rtc system call: open */
int32_t rtc_open(const uint8_t* filename);

/* function declaration for rtc system call: close */
int32_t rtc_close(int32_t fd);

/* function declaration for helper function that sets RTC frequency */
void freq_set(int32_t arg);

void rtc_test(void);

int freq_test;
int rtc_test_flag;

/* magic numbers defined for index registers and ports */
#define RTC_PORT		0x70
#define CMOS_PORT		0x71
#define INDEX_REGISTER_A	0x8A
#define	INDEX_REGISTER_B	0x8B

/* RTC IRQ on PIC defined here */
#define RTC_IRQ 8

/* magic numbers defined for setting RTC frequency */
#define MAX_RTC 1024
#define TEN 0x06
#define NINE 0x07
#define EIGHT 0x08
#define SEVEN 0x09
#define SIX 0x0A
#define FIVE 0x0B
#define FOUR 0x0C
#define THREE 0x0D
#define TWO 0x0E
#define ONE 0x0F
#define ZERO 0x00
#define THEMASK 0xF0
#define INITMASK 0x40

#endif
