//
// Created by artypoole on 18/07/24.
//

#include "PIT.h"
#include "Serial.h"
#include "ports.h"

u32 rate = 0;

volatile u32 timer_ticks = 0;


void configurePit(const u32 hz)
{
    auto& log = Serial::get();
    log.log("Initialising PIT");
    const u32 divisor = 1193180 / hz; /* Calculate our divisor */
    rate = 1193180 / divisor; // calculating back to get the real rate after integer maths
    log.log("\tConfigured PIT. Divisor: ", divisor, " rate: ", rate);
    outb(0x43, 0x36); /* Set our command byte 0x36 */
    outb(0x40, divisor & 0xFF); /* Set low byte of divisor */
    outb(0x40, divisor >> 8); /* Set high byte of divisor */
    log.log("PIT initialised");
}


void sleep(const u32 ms)
{
    auto& log = Serial::get();
    if (rate == 0)
    {
        log.log("Tried to sleep when timer is not initiated.");
        return;
    }
    timer_ticks = ms * rate / 1000; // rate is in hz, time is in ms
    log.log("Sleeping for ", ms, "ms. Ticks:", timer_ticks, " rate:", rate);
    while (timer_ticks > 0);
}

void pit_handler()
{
    // Check if sleep is still active.
    if (timer_ticks == 0) return;
    timer_ticks = timer_ticks - 1;
}
