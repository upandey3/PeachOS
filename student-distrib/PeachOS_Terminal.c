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
 *          filename
 *
 *  OUTPUT: none
 *  SOURCE: http://freesoftwaremagazine.com/articles/drivers_linux/
 *          Section : "The “memory” driver: opening the device as a file"
*/
int32_t terminal_open(const uint8_t* filename)
{
  /* Success */
  printf("Opening Terminal");
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
  printf("Closing Terminal");
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
    keyboard_terminal_index = keyboard_index;
    while(janky_spinlock_flag != 1); // voltalite flag, flag changes to 1 when ENTER_KEY is presseds

    uint32_t term_index = 0;
    uint8_t* term_buf = (uint8_t*)buf; // poointing at the same thing

    for(term_index = 0; term_index < keyboard_index && term_index < LIMIT && term_index < nbytes; term_index++)
    {
        term_buf[term_index] = keyboard_buffer[term_index]; // copying over the keyboard buffer to buf we send back
    }
    keyboard_terminal_index = term_index; // get it equal to the highest keyboard_index then next time the read function is called, we make it 0;
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
    // uint32_t size = sizeof(buf);
    // uint32_t size_2 = sizeof(uint8_t);
    // size = size / size_2;
	// cli(); // critical section we are using the keybaord buffer to write
        // temp = printf((int8_t *)buf); // print the buf that was sent in
    for(temp = 0; temp < nbytes; temp++)
    {
        uint8_t buf_char = *((uint8_t*)buf + temp);
        if(buf_char == '\n')
        {
            putc(buf_char);
            break;
        }
        putc(buf_char);
    }
    if(temp > 0)
        return temp;
    else
        return -1;
	// sti();
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
