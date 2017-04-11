#include "PeachOS_SystemCalls.h"


/*
 * Below consists of the initializing the file operation tables for certain file descriptors,
 * whenever we open a file, we make the file_jumptable pointer point to these arrays
 */

/* stdin, file operation table */
uint32_t stdin_table[4] = {(uint32_t)(terminal_open), (uint32_t)(terminal_read), (uint32_t)(dummy_function), (uint32_t)(terminal_close)};

/* stdout, file operation table */
uint32_t stdout_table[4] = {(uint32_t)(terminal_open), (uint32_t)(dummy_function), (uint32_t)(terminal_write), (uint32_t)(terminal_close)};

/* rtc, file operation table */
uint32_t rtc_table[4] = {(uint32_t)(rtc_open), (uint32_t)(rtc_read), (uint32_t)(rtc_write), (uint32_t)(rtc_close)};

/* file, file operation table */
uint32_t file_table[4] = {(uint32_t)(open_file), (uint32_t)(read_file), (uint32_t)(write_file), (uint32_t)(close_file)};

/* directory, file operation table */
uint32_t directory_table[4] = {(uint32_t)(open_directory), (uint32_t)(read_directory), (uint32_t)(write_directory), (uint32_t)(close_directory)};

/*
 * dummy_function(void)
 *
 * This function solely returns -1
 * and does nothing else
 *
 * Inputs: none
 * Outputs: returns -1
 */

int32_t halt(uint8_t status)
{
    printf("Worked\n");
    return 0;
}

int32_t execute(const uint8_t* command)
{
  /*
    // using these two temp variables for getting filesize
    uint32_t size_file_name = 0;
    uint8_t file_name[32] = {'\0'};
    uint32_t i = 0;
    uint32_t read_file = 0;

    dentry_t dir_entry;

    // iterating the passed argument array to get the size of "filename"
    while(i < 32 && command[i] != ' ')
    {
        file_name[i] = filename[i];
        i++;
        size_file_name++;
    }

    if(read_dentry_by_name(file_name, &dir_entry) == -1); // find the dir_entry from the file system
        terminal_write(1, "Didn't work", sizeof("Didn't work"));
    else
        terminal_write(1, "Worked", sizeof("Worked"));
 */
    return 0;
}

int32_t open(const uint8 t* filename)
{
    // using these two temp variables for getting filesize

    uint8_t fname[MAX_FILENAME_SIZE] = {'\0'};
    uint32_t i = 0;
    dentry_t dir_entry;

    // iterating the passed argument array to get the size of "filename"
    while(i < MAX && filename[i] != ' ')
    {
        fname[i] = filename[i];
        i++;
    }

    // find the dir_entry from the file system
    if (read_dentry_by_name(fname, &dir_entry) == -1)

        return -1;

    else {

        pcb_t * curr_pcb = get_curr_pcb();
        for (i = FIRST_FD; i <= LAST_FD; i++){

            if (curr_pcb->open_files[i].flags == NOT_AVAILABLE){

                if (i == LAST_FD) // If maximum number of files are open
                    return -1;

            } else {

                // If an available FD is found, store the FD there
                curr_pcb->open_files[i].flags = NOT_AVAILABLE;
                curr_pcb->open_files[i].file_position = OFFSET0;
                curr_pcb->filenames[i] = fname;
                break;
            }
        }
    }

     switch (dir_entry.filetype){

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

int32_t close(int32_t fd){

    int i = 0;
    uint8_t fname[MAX_FILENAME_SIZE];
    pcb_t * curr_pcb = get_curr_pcb();

    if (fd < FIRST_FD || fd > LAST_FD || curr_pcb->open_files[fd].flags == AVAILABLE)

        return -1;

    else {

        curr_pcb->open_files[fd].flags = AVAILABLE;
        curr_pcb->open_files[fd].file_position = OFFSET0;
        curr_pcb->file_jumptable[CLOSE](fd);

    }
}

int32_t read(int32_t fd, void* buf, int32_t nbytes){

// MAKE SURE READ FUNCTIONS ARE GOOD IN THE JUMP TABLES

    pcb_t * curr_pcb = get_curr_pcb();
    if (fd > LAST_FD || fd < FIRST_FD) || !buff ||
        curr_pcb->open_files[fd].flags == AVAILABLE)
        return -1;
    else
        return curr_pcb->open_files[fd].file_jumptable[READ](fd, (uint8_t *)buf, nbytes);
}

int32_t write(int32_t fd, const void* buf, int32_t nbytes){

// MAKE SURE WRITE FUNCTIONS ARE GOOD IN THE JUMP TABLES

        pcb_t * curr_pcb = get_curr_pcb();
        if (fd > LAST_FD || fd < FIRST_FD) || !buff ||
            curr_pcb->open_files[fd].flags == AVAILABLE)
            return -1;
        else
            return curr_pcb->open_files[fd].file_jumptable[WRITE](fd, (uint8_t *)buf, nbytes);
    }

}

int32_t getargs(uint8_t* buf, int32_t nbytes){

    if (!buf || !nbytes)
        return -1;

    pcb_t * curr_pcb = get_curr_pcb();
    if (nbytes < strlen(const int8_t *)curr_pcb->args))
        return -1;

    strcpy((int8_t *)buf, (const int8_t *)curr_pcb->args);

    return 0;


}
int32_t vidmap(uint8_t** screen_start){

    return -1;

}
int32_t set_handler(int32_t signum, void* handler_address){

    return -1;

}

int32_t sigreturn(void){

    return -1;

}

pcb_t * get_curr_pcb(){

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
