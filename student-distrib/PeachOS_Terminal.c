#include "PeachOS_Terminal.h"
#include "PeachOS_Keyboard.h"

#define TERMINAL_ATTRIB 0xC0
#define TERMINAL_VIDEO 0xB8000

#define LIMIT 128

uint8_t terminal_colors[MAX_TERMINAL] = {(uint8_t)0x0F, (uint8_t)0x17, (uint8_t)0x2F};
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
    uint32_t i, j;

    for(i = 0; i < MAX_TERMINAL; i++)
    {
        if(vid_flag >= 1)
        {
            vid_flag--;
            uint8_t *vterm_base_addr;
            terminal[i].terminal_video_mem = (char *)SYS_VIDMAP(&vterm_base_addr); // now we access the video memory and clear it out and color it
            printf("Terminal %d's Video Memory: %x\n", i, terminal[i].terminal_video_mem);
            vid_flag += 2;
        }

        terminal[i].terminal_index = i;

        terminal[i].terminal_x_pos = 0; // intialize them with 0
        terminal[i].terminal_y_pos = 0;


        for (j = 0; j < NUM_ROWS*NUM_COLS; j++)
        {
            *(uint8_t *)(terminal[i].terminal_video_mem + (j << 1)) = ' ';
            *(uint8_t *)(terminal[i].terminal_video_mem + (j << 1) + 1) = terminal_colors[vid_flag];
        }
    }
}

void terminal_launch(uint8_t terminal_num)
{
    uint32_t j = 0;
    vid_flag = terminal_num;

    for (j = 0; j < NUM_ROWS*NUM_COLS; j++)
    {
        *(uint8_t *)(terminal[terminal_num].terminal_video_mem + (j << 1)) = ' ';
        *(uint8_t *)(terminal[terminal_num].terminal_video_mem + (j << 1) + 1) = terminal_colors[vid_flag];
    }

    terminal[terminal_num].terminal_x_pos = 0;
    terminal[terminal_num].terminal_y_pos = 0;
    update_cursor();
    printf("Terminal %d's Video Memory: %x\n", 0, terminal[0].terminal_video_mem);
    printf("Terminal %d's Video Memory: %x\n", 1, terminal[1].terminal_video_mem);
    printf("Terminal %d's Video Memory: %x\n", 2, terminal[2].terminal_video_mem);
    // uint8_t buffer[100] = "testprint";
	// call_sys_execute(buffer);

    return;
}

void terminal_switch(uint8_t terminal_num)
{
    vid_flag = terminal_num;
    uint32_t j = 0;
    update_cursor();
    for (j = 0; j < NUM_ROWS*NUM_COLS; j++)
    {
        putc(*(uint8_t*)(terminal[terminal_num].terminal_video_mem + (j << 1)));
        *(uint8_t *)(terminal[terminal_num].terminal_video_mem + (j << 1) + 1) = terminal_colors[vid_flag];
    }
    // uint8_t *vterm_base_addr;
    // terminal[terminal_num].terminal_video_mem = (char *)SYS_VIDMAP(&vterm_base_addr); // now we access the video memory and clear it out and color it


    return;
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
