
#ifndef SYSTEM_TEST
#define SYSTEM_TEST

#include "types.h"
#include "x86_desc.h"
#include "lib.h"
#include "i8259.h"
#include "debug.h"
#include "PeachOS_PAGING.h"
#include "PeachOS_Keyboard.h"
#include "PeachOS_IDT.h"
#include "PeachOS_Interrupt.h"
#include "PeachOS_RTC.h"
#include "PeachOS_Terminal.h"
#include "PeachOS_FileSys.h"
#include "PeachOS_SystemCalls.h"

extern int32_t call_sys_halt(uint8_t status);
extern int32_t call_sys_execute(const uint8_t* command);
extern int32_t call_sys_read(int32_t fd, void* buf, int32_t nbytes);
extern int32_t call_sys_write(int32_t fd, const void* buf, int32_t nbytes);
extern int32_t call_sys_open(const uint8_t* filename);
extern int32_t call_sys_close(int32_t fd);
extern int32_t call_sys_getargs(uint8_t* buf, int32_t nbytes);
extern int32_t call_sys_vidmap(uint8_t** screen_start);
extern int32_t call_sys_set_handler(int32_t signum, void* handler_address);
extern int32_t call_sys_sigreturn(void);
//extern void testCat();

#endif
