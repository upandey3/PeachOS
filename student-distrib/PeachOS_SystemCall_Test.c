#include "PeachOS_SystemCall_Test.h"

int32_t call_sys_halt(uint8_t status)
{
    asm volatile(
    "                       \n\
    movl $1, %%eax          \n\
    movl $3, %%ebx          \n\
    int $0x80               \n\
    "
    :
    :
    : "eax"
    );

    return 12;
}

int32_t call_sys_execute(const uint8_t* command)
{
    uint8_t* buffer = (uint8_t *)command; // get the address of the buffer

    asm volatile(
    "                   \n\
    movl $2, %%eax      \n\
    movl %%ecx, %%ebx   \n\
    int $0x80           \n\
    "
    :
    : "c"(buffer)
    : "eax"
    );

    return 0;
}
