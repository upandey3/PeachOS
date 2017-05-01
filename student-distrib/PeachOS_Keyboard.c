

/* PeachOS_Keybaord.c - Uses useful keyboard library functions
 * vim:ts=4 noexpandtab
 */
#include "PeachOS_Keyboard.h"
#include "PeachOS_FileSys.h"

/* The following array is taken from
 *    http://www.osdever.net/bkerndev/Docs/keyboard.htm;
 *    https://www.daniweb.com/programming/software-development/code/216732/reading-scan-codes-from-the-keyboard
 *    https://www.win.tue.nl/~aeb/linux/kbd/scancodes-1.html
*/
static uint8_t keyboard_map[CHAR_COUNT] =
{
    /* -- NORMAL KEYS -- NO CAPS -- NO SHIFT -- */
    '\0', '\0', '1', '2', '3', '4', '5', '6', '7', '8', '9', '0',  '-', '=', '\0',
    '\0', 'q',  'w', 'e', 'r', 't', 'y', 'u', 'i', 'o', 'p', '[',  ']', '\0',
    '\0', 'a',  's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', ';', '\'', '`', '\0',
    '\\', 'z',  'x', 'c', 'v', 'b', 'n', 'm', ',', '.', '/', '\0',
    '*',  '\0', ' ', '\0'
};

static uint8_t keyboard_map_S_NC[CHAR_COUNT] =
{
    /* -- SHIFT -- NO CAPS LOCK -- OFFSET: 59 -- */
    '\0', '\0', '!', '@', '#', '$', '%', '^', '&', '*', '(', ')', '_', '+', '\0',
    '\0', 'Q', 'W', 'E', 'R', 'T', 'Y', 'U', 'I', 'O', 'P', '{', '}', '\0',
    '\0', 'A', 'S', 'D', 'F', 'G', 'H', 'J', 'K', 'L' , ':', '"', '~', '\0',
    '|', 'Z', 'X', 'C', 'V', 'B', 'N', 'M', '<', '>', '?', '\0',
    '*', '\0', ' ', '\0'
};

static uint8_t keyboard_map_S_C[CHAR_COUNT] =
{
    /* -- SHIFT -- CAPS  -- OFFSET: 118 -- */
   '\0', '\0', '!', '@', '#', '$', '%', '^', '&', '*', '(', ')', '_', '+', '\0',
   '\0', 'q', 'w', 'e', 'r', 't', 'y', 'u', 'i', 'o', 'p', '{', '}', '\0',
   '\0', 'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l' , ':', '"', '~', '\0',
   '\\', 'z', 'x', 'c', 'v', 'b', 'n', 'm', '<', '>', '?', '\0',
   '*', '\0', ' ', '\0'
};

static uint8_t keyboard_map_NS_C[CHAR_COUNT] =
{
    /* -- CAPS -- NO SHIFT -- OFFSET: 177 -- */
   '\0', '\0', '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '-', '=', '\0',
   '\0', 'Q', 'W', 'E', 'R', 'T', 'Y', 'U', 'I', 'O', 'P', '[', ']', '\0',
   '\0', 'A', 'S', 'D', 'F', 'G', 'H', 'J', 'K', 'L' , ';', '\'', '`', '\0',
   '\\', 'Z', 'X', 'C', 'V', 'B', 'N', 'M', ',', '.', '/', '\0',
   '*', '\0', ' ', '\0'
};

/* Global variable to detect if shift is pressed or not. 1 = PRESSED, 0 = NOT_PRESSED */
static int SHIFT_PRESSED = UNPRESSED;

/* Global variable to detect CAPS is ON */
static int CAPS_PRESSED = UNPRESSED;

/* CTRL Button Pressed is ON or OFF */
static int CTRL_PRESSED = UNPRESSED;

/* ALT Button Pressed is ON or OFF */
static int ALT_PRESSED_1 = UNPRESSED;

/*
 * keyboard_init
 *  DESCRIPTION:
 *          This function initializes the keyboard so it can be used
 *
 *  INPUT: none
 *
 *  OUTPUT: none
 *  SOURCE: http://arjunsreedharan.org/post/99370248137/kernel-201-lets-write-a-kernel-with-keyboard
 *          Middle of page(Starts talking about keyboard)
*/
void keyboard_init()
{
    enable_irq(KEYBOARD_IRQ);
    keyboard_index = 0;
    janky_spinlock_flag = 0;
    buffer_limit_flag = 0;
    keyboard_buffer = (uint8_t *)terminal[displayed_term].terminal_buf;

    return;
}

/*
 * keyboard_input_handler
 *  DESCRIPTION:
 *          This function will print out a charcter that was on the data register
 *
 *  INPUT: none
 *
 *  OUTPUT: none
 *  SOURCE: http://arjunsreedharan.org/post/99370248137/kernel-201-lets-write-a-kernel-with-keyboard
 *          Middle of page(Starts talking about keyboard)
 *          http://skelix.net/skelixos/tutorial05_en.html#Get_Keyboard_Work
*/
void keyboard_input_handler()
{
    /* -- MAKE SURE NO INTERRUPT HAPPEN --  */
    cli();
    /*
     * unsigned char is uint8_t. scan_input will be used to get values from
     *  the data register of keyboard.
    */
    uint8_t scan_input;

    /*
     * Getting the input from KEYBOARD_DATA_REGISTER
     * Using Polling to get the input
    */
    if((inb(KEYBOARD_CONTROL_REGISTER) & 0x01) == 1)
    {
        scan_input = inb(KEYBOARD_DATA_REGISTER);
    }
    else return;

    /*
     * As of now, scan_input contains some HEX value...
     *  We want to detect if SHIFT or CAPS or any other special character was pressed
     *    If any special characters are pressed then we need special cases to access
     *      Special parts of the keyboard_map
    */
    switch(scan_input)
    {
        /* If L_SHIFT or R_SHIFT was pressed then we need to set the SHIFT_PRESSED ON */
        case L_SHIFT_PRESSED:
            SHIFT_PRESSED = PRESSED;
            break;
        case R_SHIFT_PRESSED:
            SHIFT_PRESSED = PRESSED;
            break;

        /* If L_SHIFT or R_SHIFT was UNpressed then we need to set the SHIFT_PRESSED OFF */
        case L_SHIFT_UNPRESSED:
            SHIFT_PRESSED = UNPRESSED;
            break;
        case R_SHIFT_UNPRESSED:
            SHIFT_PRESSED = UNPRESSED;
            break;

        /* CAPS LOCK will be pressed once to Turn ON, and once again to turn OFF */
        case CAPS_LOCK:
            CAPS_PRESSED = !CAPS_PRESSED;
            break;

        /* If CTRL Pressed then turn on the CRTL_PRESSED int variable */
        case L_CTRL_PRESSED :
            CTRL_PRESSED = PRESSED;
            break;
        case L_CTRL_UNPRESSED:
            CTRL_PRESSED = UNPRESSED;
            break;

        /* If ALT Pressed then turn on the ALT_PRESSED int variable */
        case ALT_PRESSED :
            ALT_PRESSED_1 = PRESSED;
            break;
        case ALT_UNPRESSED:
            ALT_PRESSED_1 = UNPRESSED;
            break;

        /* Special cases for BACKSPACE KEY */
        case BACKSPACE:
            keyboard_backspace_key_pressed();
            break;

        /* Special case for ENTER_KEY */
        case ENTER_KEY:
            keyboard_enter_key_pressed();
            break;

        /* SKELTON CODE IN-CASE WE DO SOME SPECIAL FUNCTIONS */
        case F1_PRESSED:
            if(ALT_PRESSED_1)
            {
                sti();
                send_eoi(KEYBOARD_IRQ);
                terminal_switch(0);
            }
            break;
        case F2_PRESSED:
            if(ALT_PRESSED_1)
            {
                sti();
                send_eoi(KEYBOARD_IRQ);
                terminal_switch(1);
            }
            break;
        case F3_PRESSED:
            if(ALT_PRESSED_1)
            {
                sti();
                send_eoi(KEYBOARD_IRQ);
                terminal_switch(2);
            }
            break;
        default:
            keyboard_key_pressed(scan_input);
            break;
    }
    /*
     *  We enabled the Interrupt, so we also need to send end of interrupt signal
     *  Also need to close the critical section
    */
    send_eoi(KEYBOARD_IRQ);
    sti();
}

/*
 * keyboard_key_pressed
 *  DESCRIPTION:
 *          This function decides what to do with the key pressed.
 *          For example :- If before SHFIT was pressed then 'a' is pressed
 *                          then 'A' will be displayed on the screen
 *
 *  INPUT: A scanned input from keyboard DATA register
 *
 *  OUTPUT: Prints to screen using putc
 *  SOURCE: http://arjunsreedharan.org/post/99370248137/kernel-201-lets-write-a-kernel-with-keyboard
 *          Middle of page(Starts talking about keyboard)
 *          http://skelix.net/skelixos/tutorial05_en.html#Get_Keyboard_Work
*/
void keyboard_key_pressed(uint8_t keyboard_value)
{
    //cli();
/*
*   keyboard_map                         NORMAL_KEYS        0
*   keyboard_map_S_NC                    SHIFT_NO_CAPS      59
*   keyboard_map_S_C                     SHIFT_CAPS         118
*   keyboard_map_NS_C                    NO_SHIFT_CAPS      177
*/
    /* First sure make it's within our bounds of 59 * 4 */
    if(keyboard_value > CHAR_COUNT || keyboard_value == 0)
    {
        return;
    }

    /* -- CHECK FOR CTRL -- */
    if(CTRL_PRESSED)
    {
        uint8_t keyboard_ascii;
        keyboard_ascii = keyboard_map[keyboard_value];
        if(keyboard_ascii == 'l')
        {
            clear_screen();
        }
        if(keyboard_ascii == 's')
        {
            clear_screen();
        }
        if(keyboard_ascii == '1')
        {
            clear_screen();
            // List All Files
            print_directory();
        }
        if(keyboard_ascii == '2')
        {
            clear_screen();
            // Read File by Name
            uint8_t * char_array;
            char array[100] = "verylargetextwithverylongname.tx";
            char_array = (uint8_t* )array;
            print_file_by_name(char_array);
        }
        if(keyboard_ascii == '3')
        {
            clear_screen();
            // Read File by Index
            if (fs_ctrl_3 < 0 || fs_ctrl_3 > 16)
              fs_ctrl_3 = 0;
            print_file_by_index(fs_ctrl_3++);
        }
        if(keyboard_ascii == '4')
        {
            send_eoi(KEYBOARD_IRQ);
            sti();
            rtc_test_flag = 1;
            clear_screen();
            rtc_test();
        }
        if(keyboard_ascii == '5')
        {
            rtc_test_flag = 0;
            freq_test = 2;
            rtc_close(0);
            clear_screen();
        }

        return;
    }

    /* -- CHECK FOR SHIFT and CAPS -- */
    if(SHIFT_PRESSED && CAPS_PRESSED)
    {
        uint8_t keyboard_ascii;
        keyboard_ascii = keyboard_map_S_C[keyboard_value];

        if(buffer_limit_flag)
            return;
        else
        {
            if(terminal_flag_keyboard)
            {
                if(keyboard_index > LIMIT-1)
                    buffer_limit_flag = 1;
                if(buffer_limit_flag)
                    return;
                putc(keyboard_ascii);
                keyboard_buffer[keyboard_index] = keyboard_ascii;
                keyboard_index++;
            }
            else
            {
                putc(keyboard_ascii);
            }
        }
        return;
    }
    /* -- CHECK FOR SHIFT and NOT CAPS -- */
    else if((SHIFT_PRESSED) && (!CAPS_PRESSED))
    {
        uint8_t keyboard_ascii;
        keyboard_ascii = keyboard_map_S_NC[keyboard_value];

        if(buffer_limit_flag)
            return;
        else
        {
            if(terminal_flag_keyboard)
            {
                if(keyboard_index > LIMIT-1)
                    buffer_limit_flag = 1;
                if(buffer_limit_flag)
                    return;
                putc(keyboard_ascii);
                keyboard_buffer[keyboard_index] = keyboard_ascii;
                keyboard_index++;
            }
            else
            {
                putc(keyboard_ascii);
            }
        }
        return;
    }
    /* -- CHECK FOR NOT SHIFT and CAPS -- */
    else if((!SHIFT_PRESSED) && (CAPS_PRESSED))
    {
        uint8_t keyboard_ascii;
        keyboard_ascii = keyboard_map_NS_C[keyboard_value];

        if(buffer_limit_flag)
            return;
        else
        {
            if(terminal_flag_keyboard)
            {
                if(keyboard_index > LIMIT-1)
                    buffer_limit_flag = 1;
                if(buffer_limit_flag)
                    return;
                putc(keyboard_ascii);
                keyboard_buffer[keyboard_index] = keyboard_ascii;
                keyboard_index++;
            }
            else
            {
                putc(keyboard_ascii);
            }
        }
        return;
    }
    else
    {
        uint8_t keyboard_ascii;
        keyboard_ascii = keyboard_map[keyboard_value];

        if(buffer_limit_flag)
            return;
        else
        {
            if(terminal_flag_keyboard)
            {
                if(keyboard_index > LIMIT-1)
                    buffer_limit_flag = 1;
                if(buffer_limit_flag)
                    return;
                putc(keyboard_ascii);
                keyboard_buffer[keyboard_index] = keyboard_ascii;
                keyboard_index++;
            }
            else
            {
                putc(keyboard_ascii);
            }
        }
        return;
    }
}

/*
 * keyboard_enter_key_pressed
 *  DESCRIPTION:
 *          This function takes care scrolling and going to a new line
 *
 *  INPUT: none
 *
 *  OUTPUT: none
 *  SOURCE: Aakash C. Patel
*/
void keyboard_enter_key_pressed()
{
    if(!buffer_limit_flag)
    {
        keyboard_buffer[keyboard_index] = '\n'; // CHANGEd
        keyboard_index++;
    }
    janky_spinlock_flag = 1; // set to 1 if enter key pressed FOR TERMINAL
    newline_screen();
    return;
}

/*
 * keyboard_backspace_key_pressed
 *  DESCRIPTION:
 *          This function takes care of clearing the last displayed char
 *
 *  INPUT: none
 *
 *  OUTPUT: none
 *  SOURCE: Aakash C. Patel
*/
void
keyboard_backspace_key_pressed()
{
    if(terminal_flag_keyboard == 1 && keyboard_index == 0)
        return;
    else
    {
        backspace_screen();
        if(keyboard_index > 0)
        {
            keyboard_buffer[keyboard_index-1] = '\0';
            keyboard_index--;
            if(keyboard_index < 128)
            {
                terminal_flag_keyboard = 1;
                buffer_limit_flag = 0;
            }
        }
        if(keyboard_index <= 0)
            return;
    }
    return;
}

/*
 * empty_buffer
 *  DESCRIPTION:
 *          This function takes care of clearing out the array after we fill it all
 *
 *  INPUT: uint8_t buffer of values
 *
 *  OUTPUT: none
 *  SOURCE: Aakash C. Patel
*/
void empty_buffer(uint8_t* keyboard_buffer)
{
    int i = 0;
    for(i = 0; i < keyboard_index; i++)
    {
        keyboard_buffer[i] = '\0';
    }
    keyboard_index = 0;
    return;
}

/* --------------------- TERMINAL --------------------- */
