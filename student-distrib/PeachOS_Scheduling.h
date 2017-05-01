#ifndef _SCHEDULING_H
#define _SCHEDULING_H

#include "PeachOS_Terminal.h"
#include "PeachOS_PAGING.h"
#include "PeachOS_SystemCalls.h"
#include "types.h"
#include "x86_desc.h"


/*  This function schedules the next process, by getting the next terminal
    to run, and context switiching to it. */
void run_scheduler();
/*
This function returns the next terminal(process associated) to be
scheduled next for the scheduler
*/
int32_t get_next_terminal_running();
//Determines which terminal is being executed by the scheduler
extern volatile int32_t tasking_running_terminal;


#endif
