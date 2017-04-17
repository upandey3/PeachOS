
#include "lib.h"
#include "types.h"
#include "x86_desc.h"
#include "PeachOS_PAGING.h"
#include "PeachOS_RTC.h"
#include "PeachOS_Terminal.h"
#include "PeachOS_FileSys.h"
#include "PeachOS_SystemCalls.h"

#define READ_SIZE 4
#define ELF_EIP_START 24

/*
 * Below consists of the initializing the file operation tables for certain file descriptors,
 * whenever we open a file, we make the file_jumptable pointer point to these arrays
 * SOURCE : http://stackoverflow.com/questions/9932212/jump-table-examples-in-c
 *          http://stackoverflow.com/questions/252748/how-can-i-use-an-array-of-function-pointers
 *          http://www.geeksforgeeks.org/function-pointer-in-c/
 */
/* stdin, file operation table */
jump_table_ops stdin_table = {terminal_read, dummy_function, terminal_open, terminal_close};
/* stdout, file operation table */
jump_table_ops stdout_table = {dummy_function, terminal_write, terminal_open, terminal_close};
/* rtc, file operation table */
jump_table_ops rtc_table = {rtc_read, rtc_write, rtc_open, rtc_close};
/* file, file operation table */
jump_table_ops file_table = {read_file, write_file, open_file, close_file};
/* directory, file operation table */
jump_table_ops directory_table = {read_directory, write_directory, open_directory, close_directory};
/* closed file, file operation table */
jump_table_ops closed_table = {dummy_function, dummy_function, dummy_function, dummy_function};

uint8_t available_processes[MAX_PROCESSES] = {AVAILABLE, AVAILABLE, AVAILABLE, AVAILABLE, AVAILABLE, AVAILABLE};

uint32_t elf_eip;

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
    printf("Status is: %d \n", status);

    int index = 0;
    int retval;

    cli();

    pcb_t * current_pcb = get_curr_pcb();
    pcb_t * parent_pcb = (pcb_t*)current_pcb->parent_pcb;

    available_processes[(uint8_t)current_pcb->process_id] = AVAILABLE;

    while (index < MAX_OPEN_FILES)
    {
        retval = SYS_CLOSE(index);
        index++;
    }

    init_page(_128MB, (uint32_t)(_8MB + parent_pcb->process_id * _4MB));

    /* If we are trying to halt the last process in the terminal,
      then execute shell again
      SOURCE: https://piazza.com/class/iwy7snh02335on?cid=911 */

    tss.ss0 = KERNEL_DS;
	tss.esp0 = (uint32_t)((_8MB - (parent_pcb->process_id * _8KB)) - 4);

    sti();

    if (current_pcb->parent_process_id == current_pcb->process_id || current_pcb->parent_process_id == -1 || current_pcb->parent_process_id == 0)
    {
      printf("if reached \n");

      printf("reached after call to init_page! \n");

      SYS_EXECUTE("shell");
    }

    else
    {
      printf("else reached \n");

      asm volatile(
                 "movl %0, %%ebp;"
                 "movl %1, %%esp;"
                 "movl %2, %%eax;"
                 "jmp ret_from_halt;"
                 :
                 : "r"(parent_pcb->base_pointer), "r"(parent_pcb->stack_pointer), "r"((uint32_t) status)
                 : "esp", "ebp"
                 );
    }

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
        file_name[size_file_name] = command[i]; // OPEN FILE WITH THIS
        i++;
        size_file_name++; // so when iterating through, we do (51-49) = (i - j) = 2, file size
    }
    size_file_name++;
    file_name[size_file_name] = '\n'; // make the last charcter NULL

    while(i < TERMINAL_BUFSIZE && command[i] == ' ' && command[i] != '\0' && command[i] != '\n')
        i++; // get to the start of the argument
    while(i < TERMINAL_BUFSIZE && command[i] != '\0' && command[i] != '\n')
    {
        arg_buffer[k] = command[i]; // SEND TO THE GETARGS
        k++;
        i++;
    }
    //printf("The argument is %s\n",arg_buffer);
    /*
     * Read the file, fill in the dir_entry
     * Use the inode to send to read_data function to get the first four bytes
     * We need to check if they are DEL E L F
     */
    if(read_dentry_by_name(file_name, &dir_entry) == -1) // find the dir_entry from the file system
    {
        return -1;
    }
    else
    {
     //   terminal_write(1, "Worked\n", sizeof("Worked\n"));
     //   terminal_write(1, (uint8_t *)file_name, size_file_name + 1);
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

    uint32_t process_num = get_available_process_num(); // get the available process

    if (process_num == -1)
    {
      printf("Maximum number of processes reached, invalid command! \n");
      return -1; // CANT DO ANYTHING, MAX PROCESS REACHED
    }

    // uint32_t pcb_esp; // get esp into it
    // uint32_t pcb_ebp;

    pcb_t * pcb_new = pcb_init(process_num);
    pcb_new->base_pointer = (uint32_t)(pcb_new + _8KB - 4);
    pcb_new->stack_pointer = (uint32_t)(pcb_new + _8KB - 4);

    read_data(dir_entry.inode, ELF_EIP_START, executable_temp_buf, READ_SIZE); // 24-27 DOES MATTER, EIP

    elf_eip = *((uint32_t*) executable_temp_buf);

    init_page((uint32_t)elf_eip, (uint32_t)(_8MB + (pcb_new->process_id * _4MB)));

    //   printf("PDE: %d\n", page_directory[elf_eip >> PDBITSH].accessed);

    uint32_t file_offset = 0;

    uint8_t * shell_base = (uint8_t*)0x08048000;                              // Loading program starting 128 MB virtual address

    uint32_t file_length_size = inodes[dir_entry.inode].filelength;
    uint32_t file_length;

    /* Mapping executable into virtual address space */

    while((file_length = read_data(dir_entry.inode, file_offset, (uint8_t*)shell_base + file_offset, file_length_size - file_offset)) > 0)
    {
        file_offset = file_offset + file_length;
    }

   // printf("PDE Check: %d\n", page_directory[elf_eip >> PDBITSH].present);

    page_directory[elf_eip >> PDBITSH].accessed = 0;

    // // ESP -> EAX, EBP -> EBX
    // asm volatile(
    //     "movl %%esp, %0;"
    //     "movl %%ebp, %1;"
    //     : "=r" (pcb_esp), "=r"(pcb_ebp)
    //     :
    //     : "ebp", "esp"
    //     );
    //
    // pcb_new->parent_stack_pointer = pcb_esp;
    // pcb_new->parent_base_pointer = pcb_ebp;

    strcpy((int8_t*)pcb_new->args, (int8_t*)arg_buffer);

    tss.ss0 = KERNEL_DS; // always  the same
    tss.esp0 = (_8MB - _8KB * (pcb_new->process_id + 1)) - 4;


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
     * 14. Push'd SS
     * 15. Push'd ESP(value stored in EBP)
     * 16. Push'd EFLAGS(EAX)
     * 17. Push'd CS(EDX)
     * 18. Push'd EIP(ECX)
     * 19. Push'd Error Code
     *
     * General Sources:
     *      http://x86.renejeschke.de/html/file_module_x86_id_145.html
     *      http://www.intel.com/Assets/en_US/PDF/manual/253665.pdf INTEL MANUAL 6.4
     *      https://en.wikipedia.org/wiki/Inline_assembler
    */



    // SS, ESP, EFLAGS, CS, EIP
    // printf("works14\n");
    // printf("CURR_PCB: %x\n", pcb_new);

    asm volatile ("                 \n\
        cli                         \n\
                                    \n\
        pushl $0x2B                 \n\
        movl %1, %%eax              \n\
        pushl %%eax                 \n\
        sti                         \n\
        pushfl                      \n\
        popl %%eax                  \n\
        orl $0x3200, %%eax          \n\
        pushl %%eax                 \n\
        pushl $0x23                 \n\
        movl %0, %%eax              \n\
        pushl %%eax                 \n\
        iret                        \n\
                                    \n\
        ret_from_halt:              \n\
        "
        :
        : "r" (elf_eip), "r" (((elf_eip & 0xFFC00000) + (_4MB)) - 4)
        : "eax", "edx"
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
    //printf("SYS LINE %d", __LINE__);

    uint32_t vrft = fd;
    pcb_t *curr_pcb = get_curr_pcb();

    if (vrft > LAST_FD || curr_pcb->open_files[vrft].flags == AVAILABLE)
        return -1;
    else{
        int a;
        a = curr_pcb->open_files[vrft].file_jumptable.fd_read(vrft, buf, nbytes);
        return a;
    }
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
    uint32_t vrft = fd;

    pcb_t *curr_pcb = get_curr_pcb();
    if (vrft > LAST_FD || curr_pcb->open_files[vrft].flags == AVAILABLE)
    {
        return -1;
    }
    else
    {
        return curr_pcb->open_files[vrft].file_jumptable.fd_write(vrft, buf, nbytes);
    }
    return 0;
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
    // uint8_t fname[MAX_FILENAME_SIZE] = {'\0'};

    // // iterating the passed argument array to get the size of "filename"
    //
    // while(i < MAX_FILENAME_SIZE && filename[i] != ' ')
    // {
    //     fname[i] = filename[i];
    //     i++;
    // }

    uint32_t i = 0;
    dentry_t dir_entry;
    pcb_t *curr_pcb = get_curr_pcb();
    // printf("System_Call Line no %d\n", __LINE__);

    // find the dir_entry from the file system
    if (read_dentry_by_name(filename, &dir_entry) == -1)
        return -1;
    else
    {
        for (i = FIRST_FD; i <= LAST_FD; i++)
        {
            if (curr_pcb->open_files[i].flags == NOT_AVAILABLE)
            {
                // printf("System_Call Line no %d\n", __LINE__);

                if (i == LAST_FD) // If maximum number of files are open
                    return -1;
            }
            else
            {
                // If an available FD is found, store the FD there
                curr_pcb->open_files[i].flags = NOT_AVAILABLE;
                curr_pcb->open_files[i].file_position = OFFSET0;
                curr_pcb->open_files[i].inode = dir_entry.inode;
                break;
                //
                // for(j = 0; j < MAX_FILENAME_SIZE; j++)
                //     curr_pcb->filenames[i][j] = filename[j];
            }
        }
    }
    switch (dir_entry.filetype)
    {
        case FILE:
            if (open_file(filename) == -1)
                return -1;
            curr_pcb->open_files[i].file_jumptable = file_table;
            break;
        case DIR:
            if (open_directory(filename) == -1)
                return -1;
            curr_pcb->open_files[i].file_jumptable = directory_table;
            break;
        case RTC:
            if (rtc_open(filename) == -1)
                return -1;
            curr_pcb->open_files[i].file_jumptable = rtc_table;
            break;
        default:
            break;
     }
    //printf("The inode for '.' is %d: In open, the fd for '.' is %d\n", curr_pcb->open_files[i].inode, i);
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

/* get_curr_pcb
 *
 * Input: NONE
 *
 * Output: Address in Kernel space to start putting our PCB info
 *
 * Source: ACP
 *
*/
pcb_t * get_curr_pcb_process(uint8_t process_num)
{
    return (pcb_t*)(_8MB - (process_num + 2) * _8KB);
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


pcb_t * pcb_init(uint32_t process_num)
{
    pcb_t * parent_pcb = get_curr_pcb();
    pcb_t * curr_pcb = get_curr_pcb_process((uint8_t)process_num);

    curr_pcb->open_files[0].file_jumptable = stdin_table;
    curr_pcb->open_files[1].file_jumptable = stdout_table;

    curr_pcb->open_files[0].flags = NOT_AVAILABLE;
    curr_pcb->open_files[1].flags = NOT_AVAILABLE;
    curr_pcb->open_files[2].flags = AVAILABLE;
    curr_pcb->open_files[3].flags = AVAILABLE;
    curr_pcb->open_files[4].flags = AVAILABLE;
    curr_pcb->open_files[5].flags = AVAILABLE;
    curr_pcb->open_files[6].flags = AVAILABLE;
    curr_pcb->open_files[7].flags = AVAILABLE;

    curr_pcb->process_id = process_num;

    curr_pcb->parent_pcb = (uint32_t)parent_pcb;
    curr_pcb->base_pointer = 0;
    curr_pcb->stack_pointer = 0;

    if(parent_pcb == (pcb_t *)(_8MB - _8KB))
        curr_pcb->parent_process_id = -1;
    else
        curr_pcb->parent_process_id = parent_pcb->process_id;

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
