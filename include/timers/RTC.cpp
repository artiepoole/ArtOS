//
// Created by artypoole on 14/07/24.
//

#include "RTC.h"


u16 CURRENT_YEAR = 2000;
RTC* instance = nullptr;

u8 days_in_months[12] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
u32 RTC_ticks = 0;

u32 seconds_since_start = 0;


RTC::RTC()
{
    auto& log = Serial::get();
    log.log("Initialising RTC");
    instance = this;
    read(); // also updates current time
    char outstr[20];
    toString(outstr);
    log.log("\tRTC read. New time: ", outstr);
    setDivider(6); // also sets frequency
    log.log("RTC initialised");
}

RTC::~RTC()
{
    instance = nullptr;
}

RTC& RTC::get()
{
    return *instance;
}

tm RTC::getTime()
{
    return current_time;
}

time_t RTC::epochTime()
{
    u32 days = (current_time.tm_year -70) * 365 + current_time.tm_yday;
    return ((days * 24 + current_time.tm_hour) * 60 + current_time.tm_min) * 60 + current_time.tm_sec;
}


u8 RTC::readRegister(const u8 reg_select)
{
    outb(CMOS_SELECT, reg_select);
    return inb(CMOS_DATA);
}


u8 RTC::checkUpdating()
{
    outb(CMOS_SELECT, CMOS_STATUS_A);
    return (inb(CMOS_DATA) & 0x80);
}


tm RTC::read()
{
    while (checkUpdating()); // Make sure an update isn't in progress

    u8 second = readRegister(CMOS_SECONDS);
    u8 minute = readRegister(CMOS_MINUTES);
    u8 hour = readRegister(CMOS_HOURS);
    u8 day = readRegister(CMOS_MONTHDAY);
    u8 month = readRegister(CMOS_MONTH);
    u8 weekday = readRegister(CMOS_WEEKDAY);
    u16 year = readRegister(CMOS_YEAR);
    u8 century = 0;
    if constexpr (CMOS_CENTURY != 0)
    {
        century = readRegister(CMOS_CENTURY);
    }
    const u8 registerB = readRegister(CMOS_STATUS_B);

    if (!(registerB & 0x04))
    {
        // second = (second & 0x0F) + ((second / 16) * 10);
        second = ((second & 0xF0) >> 1) + ((second & 0xF0) >> 3) + (second & 0xf);
        // minute = (minute & 0x0F) + ((minute / 16) * 10);
        minute = ((minute & 0xF0) >> 1) + ((minute & 0xF0) >> 3) + (minute & 0xf);
        hour = ((hour & 0xF0) >> 1) + ((hour & 0xF0) >> 3) + (hour & 0xf);
        // day = (day & 0x0F) + ((day / 16) * 10);
        day = ((hour & 0xF0) >> 1) + ((day & 0xF0) >> 3) + (day & 0xf);
        // month = (month & 0x0F) + ((month / 16) * 10);
        month = ((month & 0xF0) >> 1) + ((month & 0xF0) >> 3) + (month & 0xf);
        // year = (year & 0x0F) + ((year / 16) * 10);
        year = ((year & 0xF0) >> 1) + ((year & 0xF0) >> 3) + (year & 0xf);
        if (century != 0)
        {
            century = ((century & 0xF0) >> 1) + ((century & 0xF0) >> 3) + (century & 0xf);
        }
    }

    // Check 12 hour
    if (!(registerB & 0x02) && (hour & 0x80))
    {
        hour = ((hour & 0x7F) + 12) % 24;
    }

    // Calculate the full (4-digit) year accounting for change in century
    if constexpr (CMOS_CENTURY != 0)
    {
        year += century * 100;
    }
    else
    {
        year += (CURRENT_YEAR / 100) * 100;
        if (year < CURRENT_YEAR) year += 100;
    }
    int yearday = 0;
    for (size_t i = 0; i < month; i++)
    {
        yearday += days_in_months[yearday];
    }
    if ((month >= 2) and (year % 4 == 0)) // leap year and after feb
    {
        yearday += 1;
    }
    tm time = {

        second,
        minute,
        hour,
        day,
        month,
        year - 1900,
        weekday,
        yearday,
        0
    };


    current_time = time;
    return time;
}

void RTC::increment()
{
    current_time.tm_sec++;
    if (current_time.tm_sec >= 60)
    {
        current_time.tm_sec = 0;
        current_time.tm_min++;
    }
    if (current_time.tm_min >= 60)
    {
        read(); // happens around once an hour.
    }
}


void RTC::enableInterrupts()
{
    disable_interrupts(); // disable interrupts
    outb(CMOS_SELECT, CMOS_STATUS_B);
    const u8 prev = inb(CMOS_DATA); // read the current value of register B
    outb(CMOS_SELECT, CMOS_STATUS_B);
    outb(CMOS_DATA, prev | 0x40); // write the previous value ORed with 0x40. This turns on bit 6 of register B
    enable_interrupts();
}

void RTC::disableInterrupts()
{
    disable_interrupts(); // disable interrupts
    outb(CMOS_SELECT, CMOS_STATUS_B);
    const u8 prev = inb(CMOS_DATA); // read the current value of register B
    outb(CMOS_SELECT, CMOS_STATUS_B);
    outb(CMOS_DATA, prev & ~0x40); // remove bit 6
    enable_interrupts();
}


u32 RTC::setDivider(u8 divider)
{
    //  0011b = 3 - 122 microseconds (minimum) // 8000 hz
    //  1111b = 15 - 500 milliseconds //
    //  0110b = 6 - 976.562 microseconds (default) // 1024 hz
    auto& log = Serial::get();

    hz = 32768 >> (divider - 1);
    log.log("\tSetting RTC divider. Divisor: ", static_cast<u16>(divider), " frequency: ", hz);
    divider &= 0x0F; // rate must be above 2 and not over 15
    disable_interrupts();
    outb(CMOS_SELECT, CMOS_STATUS_A);
    const u8 prev = inb(CMOS_DATA); // get initial value of register A
    outb(CMOS_SELECT, CMOS_STATUS_A);
    outb(CMOS_DATA, (prev & 0xF0) | divider); //write only our rate to A. Note, rate is the bottom 4 bits.
    enable_interrupts();
    return hz;
}

void RTC::toString(char* out_str) const
{
    u8 second = current_time.tm_sec;
    u8 minute = current_time.tm_min;
    u8 hour = current_time.tm_hour;
    u8 day = current_time.tm_mday;
    u8 month = current_time.tm_mon;
    u16 year = current_time.tm_year;

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



void rtc_handler()
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