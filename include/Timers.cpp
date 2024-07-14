//
// Created by artypoole on 13/07/24.
//

#include "Timers.h"

#include "RTC.h"

volatile u32 timer_ticks = 0;
volatile u32 clock_counter = 0;
u32 seconds_since_start = 0;
u32 rate = 0;
u32 RTC_ticks = 0;


// extern "C"
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
        log.write("Tried to sleep when timer is not initiated.");
        return;
    }

    timer_ticks = ms * rate / 1000; // rate is in hz, time is in ms

    log.write("Sleeping for ");
    log.write(ms);
    log.write("ms. Ticks: ");
    log.write(timer_ticks);
    log.write(" Rate: ");
    log.write(rate);
    log.newLine();
    while (timer_ticks > 0);
    //log.write("Exited while loop. ");
    //log.write("Remaining timer_ticks: ");
    //log.write_int(timer_ticks);
    // log.new_line();
}


void timerHandler()
{
    // Check if sleep is still active.
    if (timer_ticks == 0) return;

    timer_ticks = timer_ticks - 1;
}


void RTCHandler()
{
    // Must read register c in order to let CMOS interrupt again
    outb(CMOS_SELECT, CMOS_STATUS_C); // select register C
    inb(CMOS_DATA); // throw away contents
    RTC_ticks++;
    if (RTC_ticks % (1024) == 0)
    {
        auto & rtc = RTC::get();
        rtc.increment();
        seconds_since_start++;
    }
}
