#include "lib.h"
#include "types.h"
#include "x86_desc.h"
#include "PeachOS_PAGING.h"
#include "PeachOS_RTC.h"
#include "PeachOS_Terminal.h"
#include "PeachOS_FileSys.h"
#include "PeachOS_SystemCalls.h"

#define ELF_ADDR_SIZE 4
#define ELF_EIP_START 24
#define PCB_PTR_BYTES 4

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
uint8_t shell[ARGSIZE] = "shell";


/* System_Call : HALT
 * DESCRIPTION:
 *       This system call terminates a process, returning the specified
 *	     value to its parent process
 * System_Call_Input: Status:
 * System_Call_Output: Returns 0
 * Source: MP3 Documentation, APPENDIX B
*/
int32_t SYS_HALT(uint8_t status)
{
    cli();

    int index = NUM_ZERO;

    while (index < MAX_OPEN_FILES)
        SYS_CLOSE(index++);

    pcb_t * curr_PCB = get_pcb_by_process(terminal[tasking_running_terminal].pid);
    pcb_t * parent_PCB = get_pcb_by_process(curr_PCB->parent_process_id);

    terminal[tasking_running_terminal].pid = curr_PCB->parent_process_id;// Assign the process id for the terminal to be -1

    available_processes[(uint8_t)curr_PCB->process_id] = AVAILABLE;
    if (curr_PCB->process_id == parent_PCB->process_id || curr_PCB->process_id == NUM_ZERO || curr_PCB->parent_process_id == -1)
    {
        clear_screen();
        terminal[tasking_running_terminal].pid = -1;
        SYS_EXECUTE(shell);
    }

    /* If we are trying to halt the last process in the terminal,
      then execute shell again
      SOURCE: https://piazza.com/class/iwy7snh02335on?cid=911 */
	tss.esp0 = curr_PCB->parent_tss;
    tss.ss0 = KERNEL_DS;

    init_set_page(_128MB, _8MB + (parent_PCB->process_id * _4MB)); // Needs to change

    sti();

    asm volatile(
        "movl %0, %%ebp;"
        "movl %1, %%esp;"
        "movl %2, %%eax;"
        "jmp ret_from_halt;"
        :
        : "r"(parent_PCB->base_pointer), "r"(parent_PCB->stack_pointer), "r"((uint32_t)status)
        : "esp", "ebp", "eax"
        );
    return 0;
}

/* System_Call : EXECUTE
 * DESCRIPTION:
 *       This system call attempts to load and execute a new,
 *       handing off the processor to the new program until it terminates
 * System_Call_Input: Command, buffer that holds filename and arguments
 * System_Call_Output: Returns 0 on success, -1 on failure
 * Source: MP3 Documentation, APPENDIX B
 */
int32_t SYS_EXECUTE(const uint8_t* command)
{
    cli();
    uint8_t file_name[FILENAMESIZE] = {'\0'};
    uint32_t file_name_length = NUM_ZERO; // was 1 before
    uint8_t arg_buffer[ARGSIZE] = {'\0'};
    dentry_t dir_entry, * dir_ptr;
    dir_ptr = &dir_entry;

    /* --------  Parsing the command and collecting arguments -------- */
    file_name_length = parse_command(command, file_name, arg_buffer);

    /*----------- Read the file, and check if executable ------------ */
     if (-1 == check_executable(file_name, (uint32_t *)dir_ptr))
        return -1;

    if (strncmp((const int8_t*)file_name, (const int8_t*)shell, 5) == NUM_ZERO)
        clear_screen();

    /* ------------ Get Process ID, Create PCB -------------------- */
    uint32_t process_num = get_available_process_num(); // get the available process
    if (process_num == -1)
    {
      printf("Maximum number of processes reached, invalid command! \n");
      return -1; // CANT DO ANYTHING, MAX PROCESS REACHED
    }

    pcb_t * parent_PCB;

    if (terminal[displayed_term].pid == -1)
    {
        parent_PCB = (pcb_t *)(_8MB - _8KB);
        tasking_running_terminal = displayed_term;
    }
    else
        parent_PCB = get_pcb_by_process(terminal[displayed_term].pid);

    tasking_running_terminal = displayed_term;
    pcb_t * child_PCB = pcb_fork(process_num, parent_PCB);
    strcpy((int8_t*)child_PCB->args, (int8_t*)arg_buffer); //copy arguments to PCB

    terminal[displayed_term].pid = process_num;// Assign the process id for the terminal

    /* Get the starting instruction stored in bytes 24-27 of the executable file*/
    read_data(dir_entry.inode, ELF_EIP_START, (uint8_t *)&elf_eip, ELF_ADDR_SIZE);

    init_page(_128MB, _8MB + (child_PCB->process_id * _4MB));  // / Process for shell is at 8 MB + offset

    /* Loading the program, by copying the file to the virtual address */
    uint8_t * program_img_va = (uint8_t*)PROGRAM_IMG_ADDR;
    uint32_t filelength = inodes[dir_entry.inode].filelength;;
    read_data(dir_entry.inode, OFFSET0, program_img_va, filelength);

    /* Setting up the stack for halt*/
    asm volatile("        \n\
        movl %%ebp, %0    \n\
        movl %%esp, %1    \n\
        "
        : "=r"(child_PCB->parent_base_pointer), "=r"(child_PCB->parent_stack_pointer)
        :
        : "cc"
        );
    /* Updating parent stack and base pointers */
    if (child_PCB->parent_process_id != -1)
    {
        parent_PCB->stack_pointer = child_PCB->parent_stack_pointer;
        parent_PCB->base_pointer = child_PCB->parent_base_pointer;
    }
    /* Setting up TSS */
    child_PCB->parent_tss = tss.esp0;
    //tss.esp0 is given by parent process pcb - 4
    tss.esp0 = (_8MB - (_8KB * (child_PCB->process_id + 1))) - PCB_PTR_BYTES;
    tss.ss0 = KERNEL_DS; // always  the same

    // User program stack pointer
    uint32_t elf_eip_var = ((elf_eip & HIGHER_10_BITS_MASK) + (_4MB)) - PCB_PTR_BYTES;

    sti();
    asm volatile ("                 \n\
        cli                         \n\
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
        ret_from_halt:              \n\
        leave                       \n\
        ret                         \n\
        "
        :
        : "r" (elf_eip), "r" (elf_eip_var)
        : "eax", "edx", "esp", "ebp"
    );
    return 0;

}

uint32_t parse_command(const uint8_t* command, uint8_t* file_name, uint8_t* arg_buffer)
{
    uint32_t file_name_length = 1;
    int i = NUM_ZERO, j = NUM_ZERO, k = NUM_ZERO;
    /* ------------  Command parsing, and collecting arguments ---------------- */
    while(i < TERMINAL_BUFSIZE && command[i] == ' ')
        i++;  // increment i to get to the filename
    j = i; // for example- filename starts after 49 spaces, j = i = 49

    // parsing file name and getting the file name length;
    while(command[i] != ' ' && command[i] != '\0' && command[i] != '\n' && (i - j) < FILENAMESIZE)
    {
        file_name[file_name_length - 1] = command[i]; // -1 to get index
        i++;
        file_name_length++;
    }
    while(i < TERMINAL_BUFSIZE && command[i] == ' ' && command[i] != '\0' && command[i] != '\n')
        i++; // get to the start of the arguments
    while(i < TERMINAL_BUFSIZE && command[i] != '\0' && command[i] != '\n')
        arg_buffer[k++] = command[i++]; // collect arguments
    return file_name_length;

}
/*
 * Read the file, fill in the dir_entry
 * Use the inode to send to read_data function to get the first four bytes
 * We need to check if they are DEL E L F
 */
uint32_t check_executable(uint8_t* file_name, uint32_t* dir_ptr) {

    uint8_t exec_check[NUM_FOUR] = {ASCII_DEL, ASCII_E, ASCII_L, ASCII_F}; // del E L F stored in the buffer
    uint8_t exec_buf[NUM_FOUR];
    dentry_t * dir_entry;
    dir_entry = (dentry_t *) dir_ptr;

    if(read_dentry_by_name(file_name, dir_entry) == -1) // find the dir_entry from the file system
        return -1;
    else
    {
        // get the first four characters read from the file, if they are DEL E L F then its executable
        if(read_data(dir_entry->inode, OFFSET0, exec_buf, NUM_FOUR) == -1)
            return -1;
        /* Checking to see if it's executable, strcmp == 0 means they are same, another number means they are not */
        if(strncmp((const int8_t*)exec_buf, (const int8_t*)exec_check, NUM_FOUR) != NUM_ZERO)
            return -1;
    }
    return 0;

}

/* System_Call : READ
 * DESCRIPTION:
 *       This system call reads data from a keyboard, a file, device
 *       (RTC), or directory.
 * System_Call_Input: fd, buf, nbytes
 *      fd- file Descriptor
 *      buf- buffer to read into it
 *      nbytes- how many bytes were "read" in
 * System_Call_Output: Returns the number of bytes read
 * Source: MP3 Documentation, APPENDIX B
*/
int32_t SYS_READ(int32_t fd, void* buf, int32_t nbytes)
{
    uint32_t vrft = fd;
    if (vrft > LAST_FD || get_curr_pcb()->open_files[vrft].flags == AVAILABLE)
        return -1;
    else
        return get_curr_pcb()->open_files[vrft].file_jumptable.fd_read(vrft, buf, nbytes);
}

/* System_Call : WRITE
 * DESCRIPTION:
 *      This system call writes data to the terminal or to a device (RTC)
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
    uint32_t vrft = fd;
    if (vrft > LAST_FD || get_curr_pcb()->open_files[vrft].flags == AVAILABLE)
        return -1;
    else
        return get_curr_pcb()->open_files[vrft].file_jumptable.fd_write(vrft, buf, nbytes);
}

/* System_Call : OPEN
 * DESCRIPTION:
 *      This system call provides access to the file system, providing
 *      an unused file descriptor for the file
 * System_Call_Input: filename
 *      filename- the name of the file to open
 * System_Call_Output:
 * Source: MP3 Documentation, APPENDIX B
*/
int32_t SYS_OPEN(const uint8_t* filename)
{
    uint32_t i = 0;
    dentry_t dir_entry;
    pcb_t *curr_pcb = get_curr_pcb();

    // find the dir_entry from the file system
    if (read_dentry_by_name(filename, &dir_entry) == -1)
    {
        return -1;
    }
    else
    {
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
                curr_pcb->open_files[i].inode = dir_entry.inode;
                break;
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
    return i;
}

/* System_Call : CLOSE
 * DESCRIPTION:
 *      This system call closes the specified file descriptor and
 *      makes it available for return from later calls to open
 * System_Call_Input: fd
 *      fd- file descriptor to close within the PCB
 * System_Call_Output:
 * Source: MP3 Documentation, APPENDIX B
*/
int32_t SYS_CLOSE(int32_t fd)
{
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
 * DESCRIPTION:
 *      This system call reads the program's command line arguments
 *      into a user-level buffer.
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
    if(strlen((const int8_t *)curr_pcb->args) == 0)
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
int32_t SYS_VIDMAP(uint8_t ** screen_start)
{
    /* Map Virtual Address to Physical Address */
    if (screen_start == NULL || screen_start == (uint8_t**)_4MB)
        return -1;

    map_video_page((uint32_t)_132MB, (uint32_t)VIDEO, OFFSET0);
    *screen_start = (uint8_t*)(_132MB);
    return _132MB;
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
pcb_t * get_pcb_by_process(uint8_t process_num)
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
  * pcb_fork()
  *
  * This function initializes
  * a pcb every time a new process
  * is created
  *
  * Inputs: none
  * Outputs: returns pointer to the initialized pcb
  */
 pcb_t * pcb_fork(uint32_t process_num, pcb_t * parent_pcb)//PCB_INIT
 {
     pcb_t * child_pcb = get_pcb_by_process((uint8_t)process_num);
     child_pcb->open_files[0].file_jumptable = stdin_table;
     child_pcb->open_files[1].file_jumptable = stdout_table;
     child_pcb->open_files[0].flags = NOT_AVAILABLE;
     child_pcb->open_files[1].flags = NOT_AVAILABLE;
     child_pcb->open_files[2].flags = AVAILABLE;
     child_pcb->open_files[3].flags = AVAILABLE;
     child_pcb->open_files[4].flags = AVAILABLE;
     child_pcb->open_files[5].flags = AVAILABLE;
     child_pcb->open_files[6].flags = AVAILABLE;
     child_pcb->open_files[7].flags = AVAILABLE;
     child_pcb->process_id = process_num;
     child_pcb->parent_pcb = (uint32_t)parent_pcb;
     child_pcb->base_pointer = (uint32_t)((_8MB - (_8KB * (child_pcb->process_id + 1))) - PCB_PTR_BYTES);
     child_pcb->stack_pointer = (uint32_t)((_8MB - (_8KB * (child_pcb->process_id + 1))) - PCB_PTR_BYTES);

     if(parent_pcb == (pcb_t *)(_8MB - _8KB) || term_init_flag)//Base process
     {
         child_pcb->parent_process_id = -1;
         parent_pcb->process_id = -1;
         term_init_flag = 0;
     }
     else
         child_pcb->parent_process_id = parent_pcb->process_id;

   return child_pcb;
 }

int32_t get_available_process_num()
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
