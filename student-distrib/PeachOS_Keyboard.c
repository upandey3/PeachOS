

/* PeachOS_Keybaord.c - Uses useful keyboard library functions
 * vim:ts=4 noexpandtab
 */
#include "PeachOS_Keyboard.h"
#include "lib.h"

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
    while(1)
    {
        scan_input = inb(KEYBOARD_DATA_REGISTER);
        if((scan_input != 0) && (scan_input > 0))
            break;
    }

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
            CAPS_PRESSED = UNPRESSED;
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
            // DO SOMETHING
            //keyboard_backspace_key_pressed();
            break;

        /* Special case for ENTER_KEY */
        case ENTER_KEY:
            keyboard_enter_key_pressed();
            break;

        /* SKELTON CODE IN-CASE WE DO SOME SPECIAL FUNCTIONS */
        case F1_PRESSED:
            break;
        case F2_PRESSED:
            break;
        case F3_PRESSED:
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
    cli();
/*
*   keyboard_map                         NORMAL_KEYS        0
*   keyboard_map_S_NC                    SHIFT_NO_CAPS      59
*   keyboard_map_S_C                     SHIFT_CAPS         118
*   keyboard_map_NS_C                    NO_SHIFT_CAPS      177
*/
    /* First sure make it's within our bounds of 59 * 4 */
    if(keyboard_value > CHAR_COUNT)
    {
        sti();
        return;
    }

    /* -- CHECK FOR SHIFT and CAPS -- */
    if(SHIFT_PRESSED && CAPS_PRESSED)
    {
        uint8_t keyboard_ascii;
        keyboard_ascii = keyboard_map_S_C[keyboard_value];
        putc(keyboard_ascii);//putc(keyboard_ascii);
        sti();
        return;
    }
    /* -- CHECK FOR SHIFT and NOT CAPS -- */
    else if((SHIFT_PRESSED) && (!CAPS_PRESSED))
    {
        uint8_t keyboard_ascii;
        keyboard_ascii = keyboard_map_S_NC[keyboard_value];
        putc(keyboard_ascii);//putc(keyboard_ascii);
        sti();
        return;
    }
    /* -- CHECK FOR NOT SHIFT and CAPS -- */
    else if((!SHIFT_PRESSED) && (CAPS_PRESSED))
    {
        uint8_t keyboard_ascii;
        keyboard_ascii = keyboard_map_NS_C[keyboard_value];
        putc(keyboard_ascii);//putc(keyboard_ascii);
        sti();
        return;
    }
    else
    {
        uint8_t keyboard_ascii;
        keyboard_ascii = keyboard_map[keyboard_value];
        putc(keyboard_ascii);//putc(keyboard_ascii);
        sti();
        return;
    }
}

void keyboard_enter_key_pressed()
{
    clear();
    return;
}
