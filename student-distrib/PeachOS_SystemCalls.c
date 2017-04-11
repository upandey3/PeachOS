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
int32_t dummy_function(void)
{
  return -1;
}

int32_t halt(uint8_t status)
{
    printf("Worked\n");
    return 0;
}

int32_t execute(const uint8_t* command)
{
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

    return 0;
}

int32_t open(const uint8 t* filename)
{
    // using these two temp variables for getting filesize
    uint8_t file_name[32] = {'\0'};
    uint32_t i = 0;

    dentry_t dir_entry;

    // iterating the passed argument array to get the size of "filename"
    while(i < 32 && filename[i] != ' ')
    {
        file_name[i] = filename[i];
        i++;
    }

    read_dentry_by_name(file_name, &dir_entry); // find the dir_entry from the file system
    return 234;
}
