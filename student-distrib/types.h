/* types.h - Defines to use the familiar explicitly-sized types in this
 * OS (uint32_t, int8_t, etc.).  This is necessary because we don't want
 * to include <stdint.h> when building this OS
 * vim:ts=4 noexpandtab
 */

#ifndef _TYPES_H
#define _TYPES_H

#define NULL 0


#define _136MB 0x8800000
#define _132MB 0x8400000
#define _128MB 0x8000000
#define PROGRAM_IMG_ADDR 0x08048000
#define _12MB 0xC00000
#define _8MB 0x800000
#define _4MB 0x400000
#define _8KB 0x2000
#define _4KB 0x1000



#ifndef ASM

/* Types defined here just like in <stdint.h> */
typedef int int32_t;
typedef unsigned int uint32_t;

typedef short int16_t;
typedef unsigned short uint16_t;

typedef char int8_t;
typedef unsigned char uint8_t;

#endif /* ASM */

#endif /* _TYPES_H */
