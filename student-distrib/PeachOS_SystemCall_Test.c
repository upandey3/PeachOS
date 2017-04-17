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

//void testCat(){
//
//    int32_t fd, cnt;
//    uint8_t buf[1024];;
//    uint8_t * name = "frame0.txt";
//    pcb_t * curr_pcb = get_curr_pcb();
//    int i;
//    for (i = FIRST_FD; i <= LAST_FD; i++)
//    {
//        curr_pcb->open_files[i].flags = AVAILABLE;
//    }
//
//
//    if (-1 == (fd = SYS_OPEN(name))) {
//        printf ("file not found\n");
//	    return;
//    }
//
//    while (0 != (cnt = SYS_READ(fd, buf, 1024))) {
//        if (-1 == cnt) {
//	    printf("file read failed\n");
//	    return;
//	}
//    printf("%s", buf);
//	if (-1 == SYS_WRITE(1, buf, cnt))
//	    return;
//    }
//    printf("%s", buf);
//
//    return;
//}
