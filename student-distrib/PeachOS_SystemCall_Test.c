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
