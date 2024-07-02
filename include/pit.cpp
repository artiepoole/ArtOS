//
// Created by artypoole on 30/06/24.
//

#include "pit.h"
#include "serial.h"
#include "ports.h"


int ticks = 0;


extern "C"
void configure_pit(int hz)
{
    int divisor = 1193180 / hz;       /* Calculate our divisor */
    serial_write_string("Divisor: ");
    serial_write_int(divisor);

    outb(0x43, 0x36);             /* Set our command byte 0x36 */
    outb(0x40, divisor & 0xFF);   /* Set low byte of divisor */
    outb(0x40, divisor >> 8);     /* Set high byte of divisor */
}
