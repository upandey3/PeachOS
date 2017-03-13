#ifndef _RTC_H
#define _RTC_H

#include "types.h"

/* OSDev referenced here
*/

extern void rtc_init(void);
extern void rtc_handler(void);

#define RTC_PORT		0x70
#define CMOS_PORT		0x71
#define INDEX_REGISTER_A	0x8A
#define	INDEX_REGISTER_B	0x8B

#define RTC_IRQ 8

#endif
