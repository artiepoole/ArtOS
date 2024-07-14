//
// Created by artypoole on 30/06/24.
//

#ifndef PORTS_H
#define PORTS_H

#include "types.h"

#define PIC1		0x20		/* IO base address for master PIC */
#define PIC2		0xA0		/* IO base address for slave PIC */
#define PIC1_COMMAND	PIC1
#define PIC1_DATA	(PIC1+1)
#define PIC2_COMMAND	PIC2
#define PIC2_DATA	(PIC2+1)
#define PIC_EOI		0x20

#define KEYB_DATA		0x60 // Read only		/* IO base address for keyboard data buffer */
#define KEYB_CONTROL	0x64 // Write only		/* IO base address for keyboard control buffer */

// CMOS ports
#define CMOS_SELECT 0x70 // Write only
#define CMOS_DATA 0x71 // RW



// extern "C"
inline void outb(u16 port, u8 val)
{
    __asm__ volatile ( "outb %b0, %w1" : : "a"(val), "Nd"(port) : "memory");
    /* There's an outb %al, $imm8 encoding, for compile-time constant port numbers that fit in 8b. (N constraint).
     * Wider immediate constants would be truncated at assemble-time (e.g. "i" constraint).
     * The  outb  %al, %dx  encoding is the only option for all other cases.
     * %1 expands to %dx because  port  is a uint16_t.  %w1 could be used if we had the port number a wider C type */
}

// extern "C"
inline u8 inb(u16 port)
{
    u8 ret;
    __asm__ volatile ( "inb %w1, %b0"
                   : "=a"(ret)
                   : "Nd"(port)
                   : "memory");
    return ret;
}


#endif //PORTS_H
