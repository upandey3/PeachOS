#include "PeachOS_Terminal.h"

#define LIMIT 128
#define TERMINAL_VIDEO 0xB8000

uint8_t terminal_colors[MAX_TERMINAL] = {0xCF, 0x17, 0x2F};
uint32_t terminal_vid_mem[MAX_TERMINAL] = {_132MB + (0 * _4KB), _132MB + (1 * _4KB), _132MB + (2 * _4KB)};
uint8_t terminal_state[MAX_TERMINAL] = {AVAILABLE, AVAILABLE, AVAILABLE};

/*
 * terminal_open
 *  DESCRIPTION:
 *          This function opens the terminal, so we can read and write to it
 *
 *  INPUT: none
 *
 *  OUTPUT: none
 *  SOURCE: http://freesoftwaremagazine.com/articles/drivers_linux/
 *          Section : "loading and removing the driver in kernel space"
*/
void terminal_init()
{
    uint32_t i;
    uint32_t j;

    for(i = 0; i < MAX_TERMINAL; i++)
    {
        terminal[i].terminal_y_pos = screen_x;
        terminal[i].terminal_x_pos = screen_y; // intialize them with 0


        map_video_page(_132MB, _132MB + ((i+1) * _4KB), i+1); // mapping at 132MB + 4KB, 132MB + 8 KB, 132MB + 12KB, LEAVING 132MB OUT ON PURPOSE
        terminal[i].terminal_video_mem = (uint32_t)(_132MB + ((i+1) * _4KB)); // assigning the physical address to the terminal_video_mem

        for (j = 0; j < NUM_ROWS*NUM_COLS; j++)
        {
            *(uint8_t *)(terminal[i].terminal_video_mem + (j << 1)) = ' '; // initally clear out the screen
            *(uint8_t *)(terminal[i].terminal_video_mem + (j << 1) + 1) = terminal_colors[i]; // initally color the screen a specific color
        }
    }
}

/*
 * terminal_launch
 *  DESCRIPTION:
 *          This function launches the terminal at when kernel.c is loaded
 *
 *  INPUT: none
 *
 *  OUTPUT: none
 *  SOURCE: http://freesoftwaremagazine.com/articles/drivers_linux/
 *          Section : "loading and removing the driver in kernel space"
*/
void terminal_launch(uint8_t terminal_num)
{
    curr_term = terminal_num;

    screen_x = terminal[curr_term].terminal_x_pos; // initially when you launch terminal_x_pos is 0 so is ..._y_pos.
    screen_y = terminal[curr_term].terminal_y_pos;

    uint32_t j = 0;

    for (j = 0; j < NUM_ROWS*NUM_COLS; j++) // clear the screen again at launch
    {
        *(uint8_t *)(terminal[curr_term].terminal_video_mem + (j << 1)) = ' ';
        *(uint8_t *)(terminal[curr_term].terminal_video_mem + (j << 1) + 1) = terminal_colors[curr_term];
    }

    update_cursor();
    terminal[curr_term].terminal_x_pos = screen_x; // set the x and y for the current terminal
    terminal[curr_term].terminal_y_pos = screen_y;

    uint8_t buffer[100] = "shell"; // launch Shell
    SYS_EXECUTE(buffer);
}

/*
 * terminal_switch
 *  DESCRIPTION:
 *          This function switches the terminal when called
 *
 *  INPUT: none
 *
 *  OUTPUT: none
 *  SOURCE: http://freesoftwaremagazine.com/articles/drivers_linux/
 *          Section : "loading and removing the driver in kernel space"
*/
void terminal_switch(uint8_t terminal_num)
{
    cli();

    /* -------------------------- TAKING CARE OF OLD TERMINAL -------------------------- */
    terminal[curr_term].terminal_y_pos = screen_y; // save the current x and y, before switching the terminals
    terminal[curr_term].terminal_x_pos = screen_x;

    memcpy((void *)terminal[curr_term].terminal_video_mem, (void *)VIDEO, 2*NUM_ROWS*NUM_COLS); // copy whats on screen into terminal_video_mem

    /* -------------------------- TAKING CARE OF NEW TERMINAL -------------------------- */
    curr_term = terminal_num; // switch the terminal number

    screen_x = terminal[curr_term].terminal_x_pos; // screen_x and screen_y get updated to curr_term's x_pos and y_pos
    screen_y = terminal[curr_term].terminal_y_pos;

    memcpy((void *)VIDEO, (void *)terminal[curr_term].terminal_video_mem, 2*NUM_ROWS*NUM_COLS); // copy whats on terminal_video_mem to VIDEO(0xB8000)

    uint8_t *vterminal_base_addr; // make this point to the video_mem
    SYS_VIDMAP(&vterminal_base_addr); // remapping to video_mem

    map_video_page((uint32_t)vterminal_base_addr, (uint32_t)terminal[curr_term].terminal_video_mem, 0);

    update_cursor(); // update cursor

    sti();
}

void terminal_save_current_terminal(uint8_t terminal_num)
{

}

void terminal_restore_new_terminal(uint8_t terminal_num)
{

}

/*
 * terminal_open
 *  DESCRIPTION:
 *          This function opens the terminal, so we can read and write to it
 *
 *  INPUT:
 *          filename
 *
 *  OUTPUT: none
 *  SOURCE: http://freesoftwaremagazine.com/articles/drivers_linux/
 *          Section : "The “memory” driver: opening the device as a file"
*/
int32_t terminal_open(const uint8_t* filename)
{
  /* Success */
  // printf("Opening Terminal");
  return 0;
}

/*
 * termany_close
 *  DESCRIPTION:
 *          This function closes the terminal, so nothing can be read or written to it
 *
 *  INPUT:
 *          fd- File descriptor
 *
 *  OUTPUT: none
 *  SOURCE: http://freesoftwaremagazine.com/articles/drivers_linux/
 *          Section : "The “memory” driver: closing the device as a file"
*/
int32_t terminal_close(int32_t fd)
{
  /* Success */
  // printf("Closing Terminal");
  return 0;
}

/*
 * terminal_read
 *  DESCRIPTION:
 *          This function is used to read the keyboard buffer and send the info
 *
 *  INPUT:
 *         fd : File descriptor
 *         buf : Fill this buffer with values to send back to the USER
 *         nbytes : limit of the buffer sent in by the USER
 *
 *  OUTPUT: none
 *  SOURCE: http://freesoftwaremagazine.com/articles/drivers_linux/
 *          Section : "The “memory” driver: reading the device"
 *          Parameters taken from ece391hello.c, ece391syscall.h
*/
int32_t terminal_read(int32_t fd, void* buf, int32_t nbytes)
{
    terminal_flag_keyboard = 1; // If this is ON that means we are using the terminal functions.
    janky_spinlock_flag = 0; // janky_spinlock to get keyboard inputs
    keyboard_index = 0; // keybaord index starts out as 0 initally, so we dont take in inputs before read is used

    while(janky_spinlock_flag != 1); // voltalite flag, flag changes to 1 when ENTER_KEY is presseds

    uint32_t term_index = 0;
    uint8_t* term_buf = (uint8_t*)buf; // poointing at the same thing

    for(term_index = 0; term_index < keyboard_index && term_index < LIMIT && term_index < nbytes; term_index++)
    {
        term_buf[term_index] = keyboard_buffer[term_index]; // copying over the keyboard buffer to buf we send back
    }

    buffer_limit_flag = 0; // set the overflow flag to 0
    terminal_flag_keyboard = 0; // turn off the terminal flag

    return term_index; // return how many bytes were written on the buf
}

/*
 * terminal_write
 *  DESCRIPTION:
 *          This function is used to write to the terminal using the keboard buffer
 *
 *  INPUT:
 *         fd : File Descriptor
 *         buf : buf is already filled with values, print it out
 *         nbytes : limit of the buffer sent in by the USER
 *
 *  OUTPUT: none
 *  SOURCE: http://freesoftwaremagazine.com/articles/drivers_linux/
 *          Section : "The “memory” driver: reading the device"
 *          Parameters taken from ece391hello.c
*/
int32_t terminal_write(int32_t fd, const void* buf, int32_t nbytes)
{
    int32_t temp;
    uint8_t buffer[12] = "XXXCLEARXXX";
    if(strncmp((const int8_t*)buffer, (const int8_t*)buf, 11) == 0)
    {
        clear_screen();
        return 0;
    }
    for(temp = 0; temp < nbytes; temp++)
    {
        uint8_t buf_char = *((uint8_t*)buf + temp);
        putc(buf_char);
    }
    if(temp > 0)
        return temp;
    else
        return -1;
}

/*
 * TESTING FUNCTION
*/

int32_t terminal_test()
{
    int32_t cnt = 0;
    uint8_t buf[LIMIT];

    terminal_write(1, (uint8_t*)"Hi, what's your name? \n", LIMIT);
    cnt = terminal_read(0, buf, LIMIT);
    if (cnt == 120929)
    {
        terminal_write(1, (uint8_t*)"Can't read name from keyboard.\n", LIMIT);
        return 3;
    }
    buf[cnt] = '\n';
    terminal_write(1, (uint8_t*)"Hello, \n", LIMIT);
    terminal_write(1, buf, LIMIT);
    return 0;
}
