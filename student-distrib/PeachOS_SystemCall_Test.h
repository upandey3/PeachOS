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

#endif
