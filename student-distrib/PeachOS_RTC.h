#ifndef _RTC_H
#define _RTC_H

#include "types.h"

/* function declaration for rtc initialization */
extern void rtc_init(void);

/* function declaration for rtc interrupt handler */
extern void rtc_handler(void);

/* magic numbers defined for index registers and ports */
#define RTC_PORT		0x70
#define CMOS_PORT		0x71
#define INDEX_REGISTER_A	0x8A
#define	INDEX_REGISTER_B	0x8B

/* RTC IRQ on PIC defined here */
#define RTC_IRQ 8

#endif
