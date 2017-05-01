#include "PeachOS_Scheduling.h"

//Determines which terminal is being executed by the scheduler
volatile int32_t tasking_running_terminal = 0;

/*
* void run_scheduler()
*   DESCRIPTION
*          This function schedules the next process, by getting the next terminal
*          to run, and context switiching to it.
*   Inputs: NONE
*   Outputs: NONE
*   Return Value: NONE
 */
void run_scheduler(){


    int32_t next_term;
    // get next terminal with running task
    if ((next_term = get_next_terminal_running()) == -1)
        next_term = 0;

    // Get the process id of the next terminal---
    int32_t process_id = terminal[next_term].pid;

    //Remap to the physical address of the next process being scheduled
    init_page(_128MB, _8MB + (process_id * _4MB));

    // Getting current and currnext PCB's for context switch
    pcb_t * curr_PCB = get_pcb_by_process((uint8_t)terminal[tasking_running_terminal].pid);
    pcb_t * next_PCB = get_pcb_by_process((uint8_t)process_id);

    tasking_running_terminal = next_term;

    uint8_t * vterminal_base_addr;
    SYS_VIDMAP(&vterminal_base_addr);

    if (!terminal[next_term].displayed)
        map_video_page((uint32_t)_132MB, (uint32_t)terminal[next_term].terminal_video_mem, 0); // map the physical address of

    /* TSS bookkeping, like in SYS_EXECUTE() for the new process */
    tss.esp0 = (_8MB - (_8KB * (process_id + 1))) - 4; //curr_PCB->stack_pointer;
    tss.ss0 = KERNEL_DS;


    asm volatile(
             "movl %%esp, %%eax;"
             "movl %%ebp, %%ebx;"
             :"=a"(curr_PCB->stack_pointer), "=b"(curr_PCB->base_pointer)    /* outputs */
             :                                          /* no input */
             );

    asm volatile(
             "movl %%eax, %%esp;"
             "movl %%ebx, %%ebp;"
             :                                          /* no outputs */
             :"a"(next_PCB->stack_pointer), "b"(next_PCB->base_pointer)    /* input */
             );
    return;

}

/*
* int32_t get_next_terminal_running()
*   DESCRIPTION
*          This function returns the next terminal(process associated) to be
*          scheduled next for the scheduler
*   Inputs: NONE
*   Outputs: NONE
*   Return Value: index of the next terminal to be scheduled
*/
int32_t get_next_terminal_running(){

    int32_t term, i;
    term = tasking_running_terminal;
    for (i = 0; i < MAX_TERMINAL; i++){
        term = (term + 1) % MAX_TERMINAL;
        if (terminal[term].pid != -1)
            return term;
    }
    return -1;
}
