#include "PeachOS_Terminal.h"
#include "PeachOS_Keyboard.h"

#define TERMINAL_ATTRIB 0xC0
#define TERMINAL_VIDEO 0xB8000

#define LIMIT 128

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

    terminal.terminal_index = 0;
    for(i = 0; i < TERMINAL_BUFSIZE; i++)
    {
        terminal.terminal_buf[i] = '\0'; // clear out the terminal_buf, so we can put things in it later
    }

    terminal.terminal_x_pos = 0; // intialize them with 0
    terminal.terminal_y_pos = 0;

    // terminal.terminal_video_mem = (char *)TERMINAL_VIDEO; // now we access the video memory and clear it out and color it
    // for (i = 0; i < NUM_ROWS*NUM_COLS; i++)
    // {
    //     *(uint8_t *)(terminal.terminal_video_mem + (i << 1)) = ' ';
    //     *(uint8_t *)(terminal.terminal_video_mem + (i << 1) + 1) = TERMINAL_ATTRIB;
	// }
}

/*
 * terminal_open
 *  DESCRIPTION:
 *          This function opens the terminal, so we can read and write to it
 *
 *  INPUT:
 *          filename : STILL NEED RESEARCH
 *
 *  OUTPUT: none
 *  SOURCE: http://freesoftwaremagazine.com/articles/drivers_linux/
 *          Section : "The “memory” driver: opening the device as a file"
*/
int32_t terminal_open(const uint8_t* filename)
{
  /* Success */
  return 0;
}

/*
 * termany_close
 *  DESCRIPTION:
 *          This function closes the terminal, so nothing can be read or written to it
 *
 *  INPUT:
 *          fd : STILL NEED RESEARCH
 *
 *  OUTPUT: none
 *  SOURCE: http://freesoftwaremagazine.com/articles/drivers_linux/
 *          Section : "The “memory” driver: closing the device as a file"
*/
int32_t terminal_close(int32_t fd)
{
  /* Success */
  return 0;
}

/*
 * terminal_read
 *  DESCRIPTION:
 *          This function is used to read the keyboard buffer and send the info
 *
 *  INPUT:
 *         fd : STILL NEED RESEARCH
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

    terminal_flag_keyboard = 1;
    janky_spinlock_flag = 0;
    keyboard_index = 0;
    while(janky_spinlock_flag != 1);

    uint32_t term_index;
    uint8_t* term_buf = (uint8_t*)buf;

    for(term_index = 0; term_index < keyboard_index && term_index < LIMIT && term_index < nbytes; term_index++)
    {
        term_buf[term_index] = keyboard_buffer[term_index];
    }
    buffer_limit_flag = 0;
    terminal_flag_keyboard = 0;
	return term_index; // return how many bytes were written on the buf
}

/*
 * terminal_write
 *  DESCRIPTION:
 *          This function is used to write to the terminal using the keboard buffer
 *
 *  INPUT:
 *         fd : STILL NEED RESEARCH
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

	cli(); // critical section we are using the keybaord buffer to write
	temp = printf((int8_t *)buf); // print the buf that was sent in
	sti();

	return temp;
}

int32_t terminal_test()
{
    int32_t cnt;
    uint8_t buf[LIMIT];

    terminal_write(1, (uint8_t*)"Hi, what's your name? ", LIMIT);
    cnt = terminal_read(0, buf, LIMIT);
    if (cnt == -1)
    {
        terminal_write(1, (uint8_t*)"Can't read name from keyboard.\n", LIMIT);
        return 3;
    }
    buf[cnt] = '\0';
    terminal_write(1, (uint8_t*)"Hello, ", LIMIT);
    terminal_write(1, buf, LIMIT);

    return 0;
}
