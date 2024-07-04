//
// Created by artypoole on 30/06/24.
//

#include "pit.h"
#include "serial.h"
#include "ports.h"


u32 ticks = 0;
u32 rate;


extern "C"
void configure_pit(u32 hz)
{
    int divisor = 1193180 / hz; /* Calculate our divisor */
    rate = hz;
    serial_write_string("Configured PIT. Divisor: ");
    serial_write_int(divisor);
    serial_new_line();
    outb(0x43, 0x36); /* Set our command byte 0x36 */
    outb(0x40, divisor & 0xFF); /* Set low byte of divisor */
    outb(0x40, divisor >> 8); /* Set high byte of divisor */
}

void sleep(u32 ms)
{
    ticks = ms * 1000 / rate;
    while (ticks > 0);
}

// void pit_irq(void) /* called from Assembly */
// {
//
// }

void timer_handler(struct registers *r)
{
    /* Increment our 'tick count' */
    ticks++;

    /* Every 18 clocks (approximately 1 second), we will
    *  display a message on the screen */
    if (ticks % rate == 0)
    {
        serial_write_string("One second has passed\n");
    }
}

// void timer_install()
// {
//     /* Installs 'timer_handler' to IRQ0 */
//     irq_install_handler(0, timer_handler);
// }


