//
// Created by artypoole on 13/07/24.
//

#include "Timers.h"

#include "CMOS.h"

volatile u32 timer_ticks = 0;
volatile u32 clock_counter = 0;
u32 seconds_since_start = 0;
u32 rate = 0;


// extern "C"
void configurePit(const u32 hz)
{
    auto& log = Serial::get();
    const u32 divisor = 1193180 / hz; /* Calculate our divisor */
    rate = 1193180 / divisor; // calculating back to get the real rate after integer maths
    log.log("Configured PIT. Divisor: ", divisor, " rate: ", rate);
    outb(0x43, 0x36); /* Set our command byte 0x36 */
    outb(0x40, divisor & 0xFF); /* Set low byte of divisor */
    outb(0x40, divisor >> 8); /* Set high byte of divisor */
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

void RTC_increment()
{
    current_time.second++;
    if (current_time.second >= 60)
    {
        current_time.second = 0;
        current_time.minute++;
    }
    if (current_time.minute >= 60)
    {
        current_time.minute = 0;
        read_RTC(); // happens around once an hour.
    }
}

void timerHandler()
{
    clock_counter = clock_counter + 1; // always add one to the time counter
    if (clock_counter % rate == 0)
    {
        // approx one second has elapsed
        clock_counter = 0;
        seconds_since_start++;

        RTC_increment();
    }
    // Check if sleep is still active.
    if (timer_ticks == 0) return;

    timer_ticks = timer_ticks - 1;
}

void get_RTC_string(char* out_str)
{
    // char a = "YYYY-MM-DD HH:MM:SS\0";
    // YYYY-MM-DD HH:MM:SS - total len of
    u8 second = current_time.second;
    u8 minute = current_time.minute;
    u8 hour = current_time.hour;
    u8 day = current_time.day;
    u8 month = current_time.month;
    u16 year = current_time.year;

    out_str[4] = '-';
    out_str[7] = '-';
    out_str[10] = ' ';
    out_str[13] = ':';
    out_str[16] = ':';
    out_str[19] = '\0';

    for (size_t i = 0; i < 4; i++)
    {
        out_str[3 - i] = dec[year % 10];
        year /= 10;
    }

    for (size_t i = 0; i < 2; i++)
    {
        out_str[6 - i] = dec[month % 10];
        out_str[9 - i] = dec[day % 10];
        out_str[12 - i] = dec[hour % 10];
        out_str[15 - i] = dec[minute % 10];
        out_str[18 - i] = dec[second % 10];
        month /= 10;
        day /= 10;
        hour /= 10;
        minute /= 10;
        second /= 10;
    }
}
