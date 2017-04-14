
#include "lib.h"
#include "types.h"
#include "x86_desc.h"
#include "PeachOS_PAGING.h"
#include "PeachOS_RTC.h"
#include "PeachOS_Terminal.h"
#include "PeachOS_FileSys.h"
#include "PeachOS_SystemCalls.h"

#define USER_CS 0x0023
#define USER_DS 0x002B

/*
 * Below consists of the initializing the file operation tables for certain file descriptors,
 * whenever we open a file, we make the file_jumptable pointer point to these arrays
 * SOURCE : http://stackoverflow.com/questions/9932212/jump-table-examples-in-c
 *          http://stackoverflow.com/questions/252748/how-can-i-use-an-array-of-function-pointers
 *          http://www.geeksforgeeks.org/function-pointer-in-c/
 */
/* stdin, file operation table */
jump_table_ops stdin_table = {terminal_open, terminal_read, dummy_function, terminal_close};
/* stdout, file operation table */
jump_table_ops stdout_table = {terminal_open, dummy_function, terminal_write, terminal_close};
/* rtc, file operation table */
jump_table_ops rtc_table = {rtc_open, rtc_read, rtc_write, rtc_close};
/* file, file operation table */
jump_table_ops file_table = {open_file, read_file, write_file, close_file};
/* directory, file operation table */
jump_table_ops directory_table = {open_directory, read_directory, write_directory, close_directory};

uint8_t available_processes[MAX_PROCESSES] = {AVAILABLE, AVAILABLE};

/* System_Call : HALT
 *
 * System_Call_Input: Status
 *
 * System_Call_Output:
 *
 * Source: MP3 Documentation, APPENDIX B
*/
int32_t SYS_HALT(uint8_t status)
{
    printf("Worked\n");
    return 0;
}

/* System_Call : EXECUTE
 *
 * System_Call_Input: Command, buffer that holds filename and arguments.
 *
 * System_Call_Output:
 *
 * Source: MP3 Documentation, APPENDIX B
*/
int32_t SYS_EXECUTE(const uint8_t* command)
{
    // using these two temp variables for getting filesize
    uint32_t size_file_name = 0;
    uint8_t file_name[32] = {'\0'};
    uint8_t arg_buffer[100] = {'\0'};
    uint8_t executable_check[4] = {ASCII_DEL, ASCII_E, ASCII_L, ASCII_F}; // del E L F stored in the buffer
    uint8_t executable_temp_buf[4];
    uint32_t virtual_addr;

    uint32_t i, j, k;
    i = 0;
    j = 0;
    k = 0;
    dentry_t dir_entry;
    while(i < TERMINAL_BUFSIZE && command[i] == ' ') // increment I to get to the filename
        i++;
    j = i; // for example- filename starts after 49 spaces, j = i = 49.
    // iterating the passed argument array to get the size of "filename"
    while((i-j) < 32 && command[i] != ' ' && command[i] != '\0' && command[i] != '\n')
    {
        file_name[(i-j)] = command[i]; // OPEN FILE WITH THIS
        i++;
        size_file_name++; // so when iterating through, we do (51-49) = (i - j) = 2, file size
    }
    file_name[(i=j)] = '\0'; // make the last charcter NULL
    while(i < TERMINAL_BUFSIZE && command[i] == ' ' && command[i] != '\0' && command[i] != '\n')
        i++; // get to the start of the argument
    while(i < TERMINAL_BUFSIZE && command[i] != '\0' && command[i] != '\n')
    {
        arg_buffer[k] = command[i]; // SEND TO THE GETARGS
        k++;
        i++;
    }
    /*
	 * Read the file, fill in the dir_entry
	 * Use the inode to send to read_data function to get the first four bytes
     * We need to check if they are DEL E L F
	 */
    if(read_dentry_by_name(file_name, &dir_entry) == -1) // find the dir_entry from the file system
    {
        terminal_write(1, "Didn't work", sizeof("Didn't work"));
        return -1;
    }
    else
    {
        terminal_write(1, "Worked", sizeof("Worked"));
        terminal_write(1, (uint8_t *)file_name, size_file_name);
        // get the first four characters read from the file, if they are DEL E L F then its executable
        if(read_data(dir_entry.inode, 0, executable_temp_buf, 4) == -1)
    	{
    		return -1;
    	}
    	/* Checking to see if it's executable, strcmp == 0 means they are same, another number means they are not */
    	if(strncmp((const int8_t*)executable_temp_buf, (const int8_t*)executable_check, 4) != 0)
    	{
    		return -1;
    	}
    }

    // NOT SURE
    read_data(dir_entry.inode, 24, (uint8_t*)(&virtual_addr), sizeof(virtual_addr)); // 24 DOES MATTER, EIP
    init_page((uint32_t)virtual_addr, (uint32_t)0x800000);

    uint32_t pcb_esp; // get esp into it
    uint32_t pcb_ebp; // get ebp into it

    // ESP -> EAX, EBP -> EBX
    asm volatile("           \n\
        movl %%esp, %%eax    \n\
        movl %%ebp, %%ebx    \n\
        movl %%ebx, %%esp    \n\
        movl %%ebx, %%ebp    \n\
        "
        : "=a" (pcb_esp), "=b" (pcb_ebp)
        :
        : "ebp", "esp"
        );

    pcb_t *curr_pcb = pcb_init(pcb_esp, pcb_ebp);
    uint32_t process_num = get_available_process_num(); // get the available process
    if(process_num == -1)
        return -1; // CANT DO ANYTHING, MAX PROCESS REACHED
    curr_pcb->stack_pointer = pcb_esp;
    curr_pcb->base_pointer = pcb_ebp;
    curr_pcb->process_id = process_num;

    strcpy((int8_t*)curr_pcb->args, (int8_t*)arg_buffer);

    tss.ss0 = KERNEL_DS; // always  the same
    tss.esp0 = _8MB - _8KB * (curr_pcb->process_id) - 4;


    /* What I Did
     * 1. CLI, block interrupts
     * 2. Clear EAX, USER_DS -> EAX, moved user data segment into EAX
     * 3.4. Moved USER_DS into DS, the lower 16 bits only
     * Source for 3: https://littleosbook.github.io/, 11.3 ENTERING USER MODE
     * 5. Pushed all the flags on the stack(KERNEL)
     * 6.7. Popped teh flags and put them in EAX, after clearing it out
     * 8. Or'd EAX with 0010 0000 0000m to turn IF flag ON for user, so USER can have interrrupts occuring
     *      -> Sets up EFLAGS(EAX)
     * Source for 6: http://www.c-jump.com/CIS77/ASM/Instructions/I77_0070_eflags_bits.htm
     *  9.10. Clear out EDX, Move'd USER_CS into EDX
     *      -> Sets up CS(EDX)
     * Source for 10: ULK Section 10.3 subsection: ENTERING THE SYSTEM CALL
     * 11.12. Clear out ECX, Move'd argument 1(%0) into ECX.
     *      virtual_addr is 4 bytes long, and holds the 24-27 bytes
     *      info from EXECUTABLE FILe
     *      -> Sets up EIP(ECX)
     * Source for 12: https://en.wikipedia.org/wiki/Executable_and_Linkable_Format
     * 13. Push'd SS(same as DS)
     * Source for 13: https://web.archive.org/web/20160326062442/http://jamesmolloy.co.uk/tutorial_html/10.-User%20Mode.html
     * 14. Push'd ESP(value stored in EBP)
     * 15. Push'd EFLAGS(EAX)
     * 16. Push'd CS(EDX)
     * 17. Push'd EIP(ECX)
     * 18. Push'd Error Code
     *
     * General Sources:
     *      http://x86.renejeschke.de/html/file_module_x86_id_145.html
     *      http://www.intel.com/Assets/en_US/PDF/manual/253665.pdf INTEL MANUAL 6.4
    */

    asm volatile ("                 \n\
        cli                         \n\
        andl $0, %%eax              \n\
        movl $0x002B, %%eax        \n\
        movw %%ax, %%ds            \n\
        pushfl                      \n\
        andl $0, %%eax              \n\
        popl %%eax                  \n\
        orl $0x200, %%eax           \n\
        andl $0, %%edx              \n\
        movl $0x0023, %%edx        \n\
        andl $0, %%ecx              \n\
        movl %0, %%ecx              \n\
        pushw $0x002B              \n\
        pushl %%ebp                 \n\
        pushl %%eax                 \n\
        pushl %%edx                 \n\
        pushl %%ecx                 \n\
        pushl $0                    \n\
        iret                        \n\
        "
        :
        : "r" (virtual_addr)
        : "cc"
        );

    return 0;
}

/* System_Call : READ
 *
 * System_Call_Input: fd, buf, nbytes
 *      fd- file Descriptor
 *      buf- buffer to read into it
 *      nbytes- how many bytes were "read" in
 *
 * System_Call_Output:
 *
 * Source: MP3 Documentation, APPENDIX B
*/
int32_t SYS_READ(int32_t fd, void* buf, int32_t nbytes)
{
// MAKE SURE READ FUNCTIONS ARE GOOD IN THE JUMP TABLES
    pcb_t * curr_pcb = get_curr_pcb();
    if (fd > LAST_FD || fd < FIRST_FD || !buf ||
        curr_pcb->open_files[fd].flags == AVAILABLE)
        return -1;
    else
        return curr_pcb->open_files[fd].file_jumptable.fd_read(fd, (uint8_t *)buf, nbytes);
}

/* System_Call : WRITE
 *
 * System_Call_Input: fd, buf, nbytes
 *      fd- file descriptor
 *      buf- buffer to write from. Print to screen what's in the buffer
 *      nbytes- how many bytes to put on the screen
 *
 * System_Call_Output:
 *
 * Source: MP3 Documentation, APPENDIX B
*/
int32_t SYS_WRITE(int32_t fd, const void* buf, int32_t nbytes)
{
// MAKE SURE WRITE FUNCTIONS ARE GOOD IN THE JUMP TABLES
    pcb_t * curr_pcb = get_curr_pcb();
    if (fd > LAST_FD || fd < FIRST_FD || !buf ||
        curr_pcb->open_files[fd].flags == AVAILABLE)
        return -1;
    else
        return curr_pcb->open_files[fd].file_jumptable.fd_write(fd, buf, nbytes);
}

/* System_Call : OPEN
 *
 * System_Call_Input: filename
 *      filename- the name of the file to open
 *
 * System_Call_Output:
 *
 * Source: MP3 Documentation, APPENDIX B
*/
int32_t SYS_OPEN(const uint8_t* filename)
{
    // using these two temp variables for getting filesize
    uint8_t fname[MAX_FILENAME_SIZE] = {'\0'};
    uint32_t i = 0;
    uint32_t j = 0;
    dentry_t dir_entry;
    // iterating the passed argument array to get the size of "filename"
    while(i < MAX_FILENAME_SIZE && filename[i] != ' ')
    {
        fname[i] = filename[i];
        i++;
    }
    pcb_t *curr_pcb = get_curr_pcb();
    // find the dir_entry from the file system
    if (read_dentry_by_name(fname, &dir_entry) == -1)
        return -1;
    else
    {
        // pcb_t *curr_pcb = get_curr_pcb(); // CHANGEd
        for (i = FIRST_FD; i <= LAST_FD; i++)
        {
            if (curr_pcb->open_files[i].flags == NOT_AVAILABLE)
            {
                if (i == LAST_FD) // If maximum number of files are open
                    return -1;
            }
            else
            {
                // If an available FD is found, store the FD there
                curr_pcb->open_files[i].flags = NOT_AVAILABLE;
                curr_pcb->open_files[i].file_position = OFFSET0;
                for(j = 0; j < MAX_FILENAME_SIZE; j++)
                    curr_pcb->filenames[i][j] = fname[j];
                break;
            }
        }
    }
    switch (dir_entry.filetype)
    {
        case FILE:
            if (open_file(fname) == 200) // CHANGE THIS TO -1
                return -1;
            curr_pcb->open_files[i].inode = dir_entry.inode;
            curr_pcb->open_files[i].file_jumptable = file_table;
            break;
        case DIR:
            if (open_directory(fname) == 200) // CHANGE THIS TO -1
                return -1;
            curr_pcb->open_files[i].inode = NULL;
            curr_pcb->open_files[i].file_jumptable = directory_table;
            break;
        case RTC:
            if (rtc_open(fname) == 200) // CHANGE THIS TO -1
                return -1;
            curr_pcb->open_files[i].inode = NULL;
            curr_pcb->open_files[i].file_jumptable = rtc_table;
            break;
        default:
            break;
     }
    return i;
}
/* System_Call : CLOSE
 *
 * System_Call_Input: fd
 *      fd- file descriptor to close within the PCB
 *
 * System_Call_Output:
 *
 * Source: MP3 Documentation, APPENDIX B
*/
int32_t SYS_CLOSE(int32_t fd)
{
    // int i = 0;
    // uint8_t fname[MAX_FILENAME_SIZE]; // UNUSED
    pcb_t * curr_pcb = get_curr_pcb();
    if (fd < FIRST_FD || fd > LAST_FD || curr_pcb->open_files[fd].flags == AVAILABLE)
        return -1;
    else
    {
        curr_pcb->open_files[fd].flags = AVAILABLE;
        curr_pcb->open_files[fd].file_position = OFFSET0;
        curr_pcb->open_files[fd].file_jumptable.fd_close(fd);
    }
    return 0;
}

/* System_Call : GETARGS
 *
 * System_Call_Input: buf, nbytes
 *      buf- buffer to parse and get the arguments
 *      nbytes- how many bytes to parse
 *
 * System_Call_Output:
 *
 * Source: MP3 Documentation, APPENDIX B
*/
int32_t SYS_GETARGS(uint8_t* buf, int32_t nbytes)
{
    if (!buf || !nbytes)
        return -1;
    pcb_t * curr_pcb = get_curr_pcb();
    if (nbytes < strlen((const int8_t *)curr_pcb->args))
        return -1;
    strcpy((int8_t *)buf, (const int8_t *)curr_pcb->args);
    return 0;
}

/* System_Call : VIDMAP
 *
 * System_Call_Input: screen_start
 *
 * System_Call_Output:
 *
 * Source: MP3 Documentation, APPENDIX B
*/
int32_t SYS_VIDMAP(uint8_t** screen_start)
{
    return -1;
}

/* System_Call : SET_HANDLER
 *
 * System_Call_Input: signum, handler_address
 *
 * System_Call_Output:
 *
 * Source: MP3 Documentation, APPENDIX B
*/
int32_t SYS_SET_HANDLER(int32_t signum, void* handler_address)
{
    return -1;
}

/* System_Call : SIGRETURN
 *
 * System_Call_Input: void
 *
 * System_Call_Output:
 *
 * Source: MP3 Documentation, APPENDIX B
*/
int32_t SYS_SIGRETURN(void)
{
    return -1;
}


/* get_curr_pcb
 *
 * Input: NONE
 *
 * Output: Address in Kernel space to start putting our PCB info
 *
 * Source: https://www.usna.edu/Users/cs/aviv/classes/ic221/s16/lec/21/lec.html
 *         http://proquest.safaribooksonline.com/book/operating-systems-and-server-administration/linux/0596005652/3dot-processes/understandlk-chp-3-sect-2?bookview=search&query=Process%20Control%20Block
 *         https://www.tutorialspoint.com/operating_system/os_processes.htm
 *         https://www.slideshare.net/imdadmanik/03-processes
*/
pcb_t * get_curr_pcb()
{
    pcb_t * ret;
    asm volatile ("       \n\
        andl %%esp, %%ecx \n\
        "
        : "=c" (ret)
        : "c" (LOWER_13_BITS_MASK)
        : "cc"
        );
    return ret;
}


/*
 * dummy_function(void)
 *
 * This function solely returns -1
 * and does nothing else
 *
 * Inputs: none
 * Outputs: returns -1
 */
 int32_t dummy_function()
 {
    return -1;
 }


/*
 * pcb_init()
 *
 * This function initializes
 * a pcb every time a new process
 * is created
 *
 * Inputs: none
 * Outputs: returns pointer to the initialized pcb
 */


pcb_t *pcb_init(uint32_t curr_esp, uint32_t curr_ebp)
{
  pcb_t *curr_pcb = get_curr_pcb();
  curr_pcb->open_files[0].file_jumptable = stdin_table;
  curr_pcb->open_files[1].file_jumptable = stdout_table;
  curr_pcb->process_id = -1;
  //curr_pcb->args[] = {'\0'};
  curr_pcb->stack_pointer = curr_ebp - _8KB;
  curr_pcb->base_pointer = curr_ebp - _8KB;
  curr_pcb->parent_process_id = -2;
  curr_pcb->parent_stack_pointer = curr_esp;
  curr_pcb->parent_base_pointer = curr_ebp;
  return curr_pcb;
}

uint32_t get_available_process_num()
{
    uint32_t j = 0;
    for(j = 0; j < MAX_PROCESSES; j++)
    {
        if(available_processes[j] == AVAILABLE)
        {
            available_processes[j] = NOT_AVAILABLE;
            return j;
        }
    }
    return -1;
}
