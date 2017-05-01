/* lib.c - Some basic library functions (printf, strlen, etc.)
 * vim:ts=4 noexpandtab
 */

#include "lib.h"


int screen_x = 0;
int screen_y = 0;
uint8_t displayed_term = 0;
char* video_mem = (char *)VIDEO;

/*
* void clear(void);
*   Inputs: void
*   Return Value: none
*	Function: Clears video memory
*/

void
clear(void)
{
    int32_t i;
    for(i=0; i<NUM_ROWS*NUM_COLS; i++) {
        *(uint8_t *)(video_mem + (i << 1)) = ' ';
        *(uint8_t *)(video_mem + (i << 1) + 1) = terminal_colors[displayed_term];
    }
}

/*
* clear_screen(void);
*   Inputs: void
*   Return Value: none
*	Function: Clears video memory
*/
void
clear_screen(void)
{
    int32_t i; // Same as clear(), but we make screen_x and screen_y 0
    for(i=0; i<NUM_ROWS*NUM_COLS; i++) {
        *(uint8_t *)(video_mem + (i << 1)) = ' ';
        *(uint8_t *)(video_mem + (i << 1) + 1) = terminal_colors[displayed_term];
    }
    screen_y = 0;
    screen_x = 0;
    update_cursor();
}

/*
* newline_screen(void);
*   Inputs: void
*   Return Value: none
*	Function: Clears video memory
*/
void
newline_screen(void)
{
    update_cursor();
    if(screen_y == NUM_ROWS -1) //if we are at the bottom edge then we "scroll"
    {
        int32_t i; // if we are at the right most edge, we goto a new line
        for(i = 0; i < (NUM_ROWS - 1) * NUM_COLS; i++)
        {
            *(uint8_t *)(video_mem + (i << 1)) = *(uint8_t *)(video_mem + ((i + NUM_COLS) << 1));
            *(uint8_t *)(video_mem + (i << 1) + 1) = terminal_colors[displayed_term]; // implementing scrolling
        }                                                    // copying over data from previous line
        for(i = (NUM_ROWS-1)*NUM_COLS; i < (NUM_ROWS)*NUM_COLS; i++)
        {
            *(uint8_t *)(video_mem + (i << 1)) = ' ';
            *(uint8_t *)(video_mem + (i << 1) + 1) = terminal_colors[displayed_term]; // clearing out the last line, becuase we "Scrolled"
        }
        screen_x = 0;
        update_cursor();
    }
    else
    {
        screen_x = 0; // if we arent at the bottom edge then just increment the screen x and screen y
        screen_y++;
        update_cursor();
    }
    update_cursor();
}

/*
* backspace_screen(void);
*   Inputs: void
*   Return Value: none
*	Function: Clears video memory
*/
void
backspace_screen(void)
{
    update_cursor();
    if((screen_x == 0 && screen_y == 0)) // if we are at the beginning of the screen dont do anything
        return;
    if(screen_x) // if we screen_x > 0 then we delete horizontally
    {
        screen_x--;
        *(uint8_t *)(video_mem + (((screen_y * NUM_COLS) + screen_x) << 1) + 1) = terminal_colors[displayed_term];
        *(uint8_t *)(video_mem + (((screen_y * NUM_COLS) + screen_x) << 1)) = ' ';
    }
    else // if screen_x is 0 then we delete verticall, then horizontally again
    {
        screen_y--;
        screen_x = NUM_COLS - 1;
        *(uint8_t *)(video_mem + (((screen_y * NUM_COLS) + screen_x) << 1) + 1) = terminal_colors[displayed_term];
        *(uint8_t *)(video_mem + (((screen_y * NUM_COLS) + screen_x) << 1)) = ' ';
    }
    update_cursor(); // update cursor becuase screen_x or screen_y was changed
}

/*
* update_cursor(row, col);
*   Inputs: row and col of where we are
*   Return Value: none
*	Function: Clears video memory
*   Source: http://wiki.osdev.org/Text_Mode_Cursor
*           http://www.osdever.net/FreeVGA/vga/textcur.htm
*/
void
update_cursor(void)
{
   uint32_t position = (screen_y * NUM_COLS) + screen_x; // get the position of where you are on the screen

   // cursor LOW port to vga INDEX register
   outb(0x0F, LOW_PORT_VGA);
   outb((uint8_t)(position&0xFF), LOW_PORT_VGA_2);
   // cursor HIGH port to vga INDEX register
   outb(0x0E, LOW_PORT_VGA);
   outb((uint8_t)((position>>8)&0xFF), LOW_PORT_VGA_2);
}


/*
* Current_x;
*   Inputs: none
*   Return Value: none
*	Function: gets current
*/
int
x_position()
{
  return screen_x;
}

/* Standard printf().
 * Only supports the following format strings:
 * %%  - print a literal '%' character
 * %x  - print a number in hexadecimal
 * %u  - print a number as an unsigned integer
 * %d  - print a number as a signed integer
 * %c  - print a character
 * %s  - print a string
 * %#x - print a number in 32-bit aligned hexadecimal, i.e.
 *       print 8 hexadecimal digits, zero-padded on the left.
 *       For example, the hex number "E" would be printed as
 *       "0000000E".
 *       Note: This is slightly different than the libc specification
 *       for the "#" modifier (this implementation doesn't add a "0x" at
 *       the beginning), but I think it's more flexible this way.
 *       Also note: %x is the only conversion specifier that can use
 *       the "#" modifier to alter output.
 * */
int32_t
printf(int8_t *format, ...)
{
	/* Pointer to the format string */
	int8_t* buf = format;

    update_cursor();
	/* Stack pointer for the other parameters */
	int32_t* esp = (void *)&format;
	esp++;

	while(*buf != '\0') {
		switch(*buf) {
			case '%':
				{
					int32_t alternate = 0;
					buf++;

format_char_switch:
					/* Conversion specifiers */
					switch(*buf) {
						/* Print a literal '%' character */
						case '%':
							putc('%');
							break;

						/* Use alternate formatting */
						case '#':
							alternate = 1;
							buf++;
							/* Yes, I know gotos are bad.  This is the
							 * most elegant and general way to do this,
							 * IMHO. */
							goto format_char_switch;

						/* Print a number in hexadecimal form */
						case 'x':
							{
								int8_t conv_buf[64];
								if(alternate == 0) {
									itoa(*((uint32_t *)esp), conv_buf, 16);
									puts(conv_buf);
								} else {
									int32_t starting_index;
									int32_t i;
									itoa(*((uint32_t *)esp), &conv_buf[8], 16);
									i = starting_index = strlen(&conv_buf[8]);
									while(i < 8) {
										conv_buf[i] = '0';
										i++;
									}
									puts(&conv_buf[starting_index]);
								}
								esp++;
							}
							break;

						/* Print a number in unsigned int form */
						case 'u':
							{
								int8_t conv_buf[36];
								itoa(*((uint32_t *)esp), conv_buf, 10);
								puts(conv_buf);
								esp++;
							}
							break;

						/* Print a number in signed int form */
						case 'd':
							{
								int8_t conv_buf[36];
								int32_t value = *((int32_t *)esp);
								if(value < 0) {
									conv_buf[0] = '-';
									itoa(-value, &conv_buf[1], 10);
								} else {
									itoa(value, conv_buf, 10);
								}
								puts(conv_buf);
								esp++;
							}
							break;

						/* Print a single character */
						case 'c':
							putc( (uint8_t) *((int32_t *)esp) );
							esp++;
							break;

						/* Print a NULL-terminated string */
						case 's':
							puts( *((int8_t **)esp) );
							esp++;
							break;

						default:
							break;
					}

				}
				break;

			default:
				putc(*buf);
				break;
		}
		buf++;
	}

	return (buf - format);
}

/*
* int32_t puts(int8_t* s);
*   Inputs: int_8* s = pointer to a string of characters
*   Return Value: Number of bytes written
*	Function: Output a string to the console
*/

int32_t
puts(int8_t* s)
{
	register int32_t index = 0;
	while(s[index] != '\0') {
		putc(s[index]);
		index++;
	}

	return index;
}

/*
* void putc(uint8_t c);
*   Inputs: uint_8* c = character to print
*   Return Value: void
*	Function: Output a character to the console
*/

void
putc(uint8_t c)
{
    update_cursor();
    if(c == '\n' || c == '\r')
    {
        newline_screen();
        update_cursor();
    }
    else
    {
        *(uint8_t *)(video_mem + ((NUM_COLS*screen_y + screen_x) << 1)) = c;
        *(uint8_t *)(video_mem + ((NUM_COLS*screen_y + screen_x) << 1) + 1) = terminal_colors[displayed_term];
        screen_x++;
        if(screen_x == NUM_COLS)
        {
            newline_screen();
            update_cursor();
        }
        screen_y = (screen_y + (screen_x / NUM_COLS)) % NUM_ROWS;
    }
    update_cursor();
}

/*
* void terminal_putc(uint8_t character);
*   Inputs: uint_8* c = character to print
*   Return Value: void
*	Function: Output a character to the physical memory of tasking_running_terminal
*/
void
terminal_putc(uint8_t character)
{
    // if newline or tab we need to scroll down
    if(character == '\n' || character == '\r')
    {
        term_newline();
    }
    else // put the character in the appropiate location, and change the color
    {
        *(uint8_t *)(terminal[tasking_running_terminal].terminal_video_mem + ((NUM_COLS*terminal[tasking_running_terminal].terminal_y_pos + terminal[tasking_running_terminal].terminal_x_pos) << 1)) = character;
        *(uint8_t *)(terminal[tasking_running_terminal].terminal_video_mem + ((NUM_COLS*screen_y + screen_x) << 1) + 1) = terminal_colors[tasking_running_terminal];
        terminal_screen_reset(terminal[tasking_running_terminal].terminal_x_pos + 1, terminal[tasking_running_terminal].terminal_y_pos);
    }
}


/*
* void terminal_screen_reset(uint32_t term_x, uint32_t term_y)
*   Inputs: uint32_t term_x, uint32_t term_y = change the screen here
*   Return Value: void
*	Function: scrolls_up if the values for term_x or term_y are too big
*/
void
terminal_screen_reset(uint32_t term_x, uint32_t term_y)
{
    // if term_x is appropiate then assign it value
	if (term_x < NUM_COLS)
		terminal[tasking_running_terminal].terminal_x_pos = term_x;
	else // if not then goto newline
    {
		term_newline();
		return;
	}
    // if term_y is appropiate then assgin it value
	if (term_y < NUM_ROWS)
		terminal[tasking_running_terminal].terminal_y_pos = term_y;
	else // if not we need to scroll_up possibly
    {
		term_newline_screen();
		terminal[tasking_running_terminal].terminal_y_pos = NUM_ROWS - 1;
	}
}

/*
* void term_newline(void)
*   Inputs: none
*   Return Value: void
*	Function: scrolls to new_line
*/
void term_newline(void)
{
	terminal_screen_reset(ZERO, terminal[tasking_running_terminal].terminal_y_pos + 1);
}

/*
* void term_newline_screen()
*   Inputs: none
*   Return Value: void
*	Function: scrolls to new_line
*/
void term_newline_screen()
{
	int32_t term_x;
	int32_t term_y;

	int32_t pos_old;
	int32_t pos_new;

	// incrementing term_x and term_y
	for (term_y = 0; term_y < NUM_ROWS-1; term_y++)
    {   // save the old_pos and get new_pos for it
		for (term_x = 0; term_x < NUM_COLS; term_x++)
        { // assign the character depending on the new_pos
			pos_old = NUM_COLS*(term_y+1) + term_x;
			pos_new = NUM_COLS*term_y + term_x;
			*(uint8_t *)(terminal[tasking_running_terminal].terminal_video_mem + (pos_new << 1)) = *(uint8_t *)(terminal[tasking_running_terminal].terminal_video_mem + (pos_old << 1));
		}
	}

	// once done, we need to clear out the bottom of the screen
	for (term_x = 0; term_x < NUM_COLS; term_x++)
    { // incrementing the x value in the bottom , and then clearning it out
		pos_new = NUM_COLS*(NUM_ROWS-1) + term_x;
		*(uint8_t *)(terminal[tasking_running_terminal].terminal_video_mem + (pos_new << 1)) = ' ';
	}
	terminal_screen_reset(ZERO, terminal[tasking_running_terminal].terminal_y_pos);
}

/*
* int8_t* itoa(uint32_t value, int8_t* buf, int32_t radix);
*   Inputs: uint32_t value = number to convert
*			int8_t* buf = allocated buffer to place string in
*			int32_t radix = base system. hex, oct, dec, etc.
*   Return Value: number of bytes written
*	Function: Convert a number to its ASCII representation, with base "radix"
*/

int8_t*
itoa(uint32_t value, int8_t* buf, int32_t radix)
{
	static int8_t lookup[] = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ";

	int8_t *newbuf = buf;
	int32_t i;
	uint32_t newval = value;

	/* Special case for zero */
	if(value == 0) {
		buf[0]='0';
		buf[1]='\0';
		return buf;
	}

	/* Go through the number one place value at a time, and add the
	 * correct digit to "newbuf".  We actually add characters to the
	 * ASCII string from lowest place value to highest, which is the
	 * opposite of how the number should be printed.  We'll reverse the
	 * characters later. */
	while(newval > 0) {
		i = newval % radix;
		*newbuf = lookup[i];
		newbuf++;
		newval /= radix;
	}

	/* Add a terminating NULL */
	*newbuf = '\0';

	/* Reverse the string and return */
	return strrev(buf);
}

/*
* int8_t* strrev(int8_t* s);
*   Inputs: int8_t* s = string to reverse
*   Return Value: reversed string
*	Function: reverses a string s
*/

int8_t*
strrev(int8_t* s)
{
	register int8_t tmp;
	register int32_t beg=0;
	register int32_t end=strlen(s) - 1;

	while(beg < end) {
		tmp = s[end];
		s[end] = s[beg];
		s[beg] = tmp;
		beg++;
		end--;
	}

	return s;
}

/*
* uint32_t strlen(const int8_t* s);
*   Inputs: const int8_t* s = string to take length of
*   Return Value: length of string s
*	Function: return length of string s
*/

uint32_t
strlen(const int8_t* s)
{
	register uint32_t len = 0;
	while(s[len] != '\0')
		len++;

	return len;
}

/*
* void* memset(void* s, int32_t c, uint32_t n);
*   Inputs: void* s = pointer to memory
*			int32_t c = value to set memory to
*			uint32_t n = number of bytes to set
*   Return Value: new string
*	Function: set n consecutive bytes of pointer s to value c
*/

void*
memset(void* s, int32_t c, uint32_t n)
{
	c &= 0xFF;
	asm volatile("                  \n\
			.memset_top:            \n\
			testl   %%ecx, %%ecx    \n\
			jz      .memset_done    \n\
			testl   $0x3, %%edi     \n\
			jz      .memset_aligned \n\
			movb    %%al, (%%edi)   \n\
			addl    $1, %%edi       \n\
			subl    $1, %%ecx       \n\
			jmp     .memset_top     \n\
			.memset_aligned:        \n\
			movw    %%ds, %%dx      \n\
			movw    %%dx, %%es      \n\
			movl    %%ecx, %%edx    \n\
			shrl    $2, %%ecx       \n\
			andl    $0x3, %%edx     \n\
			cld                     \n\
			rep     stosl           \n\
			.memset_bottom:         \n\
			testl   %%edx, %%edx    \n\
			jz      .memset_done    \n\
			movb    %%al, (%%edi)   \n\
			addl    $1, %%edi       \n\
			subl    $1, %%edx       \n\
			jmp     .memset_bottom  \n\
			.memset_done:           \n\
			"
			:
			: "a"(c << 24 | c << 16 | c << 8 | c), "D"(s), "c"(n)
			: "edx", "memory", "cc"
			);

	return s;
}

/*
* void* memset_word(void* s, int32_t c, uint32_t n);
*   Inputs: void* s = pointer to memory
*			int32_t c = value to set memory to
*			uint32_t n = number of bytes to set
*   Return Value: new string
*	Function: set lower 16 bits of n consecutive memory locations of pointer s to value c
*/

/* Optimized memset_word */
void*
memset_word(void* s, int32_t c, uint32_t n)
{
	asm volatile("                  \n\
			movw    %%ds, %%dx      \n\
			movw    %%dx, %%es      \n\
			cld                     \n\
			rep     stosw           \n\
			"
			:
			: "a"(c), "D"(s), "c"(n)
			: "edx", "memory", "cc"
			);

	return s;
}

/*
* void* memset_dword(void* s, int32_t c, uint32_t n);
*   Inputs: void* s = pointer to memory
*			int32_t c = value to set memory to
*			uint32_t n = number of bytes to set
*   Return Value: new string
*	Function: set n consecutive memory locations of pointer s to value c
*/

void*
memset_dword(void* s, int32_t c, uint32_t n)
{
	asm volatile("                  \n\
			movw    %%ds, %%dx      \n\
			movw    %%dx, %%es      \n\
			cld                     \n\
			rep     stosl           \n\
			"
			:
			: "a"(c), "D"(s), "c"(n)
			: "edx", "memory", "cc"
			);

	return s;
}

/*
* void* memcpy(void* dest, const void* src, uint32_t n);
*   Inputs: void* dest = destination of copy
*			const void* src = source of copy
*			uint32_t n = number of byets to copy
*   Return Value: pointer to dest
*	Function: copy n bytes of src to dest
*/

void*
memcpy(void* dest, const void* src, uint32_t n)
{
	asm volatile("                  \n\
			.memcpy_top:            \n\
			testl   %%ecx, %%ecx    \n\
			jz      .memcpy_done    \n\
			testl   $0x3, %%edi     \n\
			jz      .memcpy_aligned \n\
			movb    (%%esi), %%al   \n\
			movb    %%al, (%%edi)   \n\
			addl    $1, %%edi       \n\
			addl    $1, %%esi       \n\
			subl    $1, %%ecx       \n\
			jmp     .memcpy_top     \n\
			.memcpy_aligned:        \n\
			movw    %%ds, %%dx      \n\
			movw    %%dx, %%es      \n\
			movl    %%ecx, %%edx    \n\
			shrl    $2, %%ecx       \n\
			andl    $0x3, %%edx     \n\
			cld                     \n\
			rep     movsl           \n\
			.memcpy_bottom:         \n\
			testl   %%edx, %%edx    \n\
			jz      .memcpy_done    \n\
			movb    (%%esi), %%al   \n\
			movb    %%al, (%%edi)   \n\
			addl    $1, %%edi       \n\
			addl    $1, %%esi       \n\
			subl    $1, %%edx       \n\
			jmp     .memcpy_bottom  \n\
			.memcpy_done:           \n\
			"
			:
			: "S"(src), "D"(dest), "c"(n)
			: "eax", "edx", "memory", "cc"
			);

	return dest;
}

/*
* void* memmove(void* dest, const void* src, uint32_t n);
*   Inputs: void* dest = destination of move
*			const void* src = source of move
*			uint32_t n = number of byets to move
*   Return Value: pointer to dest
*	Function: move n bytes of src to dest
*/

/* Optimized memmove (used for overlapping memory areas) */
void*
memmove(void* dest, const void* src, uint32_t n)
{
	asm volatile("                  \n\
			movw    %%ds, %%dx      \n\
			movw    %%dx, %%es      \n\
			cld                     \n\
			cmp     %%edi, %%esi    \n\
			jae     .memmove_go     \n\
			leal    -1(%%esi, %%ecx), %%esi    \n\
			leal    -1(%%edi, %%ecx), %%edi    \n\
			std                     \n\
			.memmove_go:            \n\
			rep     movsb           \n\
			"
			:
			: "D"(dest), "S"(src), "c"(n)
			: "edx", "memory", "cc"
			);

	return dest;
}

/*
* int32_t strncmp(const int8_t* s1, const int8_t* s2, uint32_t n)
*   Inputs: const int8_t* s1 = first string to compare
*			const int8_t* s2 = second string to compare
*			uint32_t n = number of bytes to compare
*	Return Value: A zero value indicates that the characters compared
*					in both strings form the same string.
*				A value greater than zero indicates that the first
*					character that does not match has a greater value
*					in str1 than in str2; And a value less than zero
*					indicates the opposite.
*	Function: compares string 1 and string 2 for equality
*/

int32_t
strncmp(const int8_t* s1, const int8_t* s2, uint32_t n)
{
	int32_t i;
	for(i=0; i<n; i++) {
		if( (s1[i] != s2[i]) ||
				(s1[i] == '\0') /* || s2[i] == '\0' */ ) {

			/* The s2[i] == '\0' is unnecessary because of the short-circuit
			 * semantics of 'if' expressions in C.  If the first expression
			 * (s1[i] != s2[i]) evaluates to false, that is, if s1[i] ==
			 * s2[i], then we only need to test either s1[i] or s2[i] for
			 * '\0', since we know they are equal. */

			return s1[i] - s2[i];
		}
	}
	return 0;
}

/*
* int8_t* strcpy(int8_t* dest, const int8_t* src)
*   Inputs: int8_t* dest = destination string of copy
*			const int8_t* src = source string of copy
*   Return Value: pointer to dest
*	Function: copy the source string into the destination string
*/

int8_t*
strcpy(int8_t* dest, const int8_t* src)
{
	int32_t i=0;
	while(src[i] != '\0') {
		dest[i] = src[i];
		i++;
	}

	dest[i] = '\0';
	return dest;
}

/*
* int8_t* strcpy(int8_t* dest, const int8_t* src, uint32_t n)
*   Inputs: int8_t* dest = destination string of copy
*			const int8_t* src = source string of copy
*			uint32_t n = number of bytes to copy
*   Return Value: pointer to dest
*	Function: copy n bytes of the source string into the destination string
*/

int8_t*
strncpy(int8_t* dest, const int8_t* src, uint32_t n)
{
	int32_t i=0;
	while(src[i] != '\0' && i < n) {
		dest[i] = src[i];
		i++;
	}

	while(i < n) {
		dest[i] = '\0';
		i++;
	}

	return dest;
}
