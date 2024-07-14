//
// Created by artypoole on 14/07/24.
//

#include "CMOS.h"

#include "Serial.h"

RTC_t current_time;
u16 CURRENT_YEAR = 2000;

// Registers
#define CMOS_REGISTER_FLOPPY 0x10 // https://wiki.osdev.org/CMOS#Accessing_CMOS_Registers
#define CMOS_SECONDS 0x00 // 0-59
#define CMOS_MINUTES 0x02 // 0-59
#define CMOS_HOURS 0x04 // 0-24 or 1-12 with highest bit set if after noon
#define CMOS_WEEKDAY 0x06 // 1-7, 1 is sunday
#define CMOS_MONTHDAY 0x07 // 1-31
#define CMOS_MONTH 0x08 // 1-12
#define CMOS_YEAR 0x09 // 0-99
#define CMOS_CENTURY 0x32 // 19-20?  might not work
#define CMOS_STATUS_A 0x0A   // 0x80 bit is "update in progress" flag
#define CMOS_STATUS_B 0x0B

u8 get_from_cmos(const u8 reg_select)
{
    outb(CMOS_SELECT, reg_select);
    return inb(CMOS_DATA);
}


u8 get_update_in_progress_flag()
{
    outb(CMOS_SELECT, CMOS_STATUS_A);
    return (inb(CMOS_DATA) & 0x80);
}

void read_RTC()
{
    auto & log = Serial::get();

    while (get_update_in_progress_flag()); // Make sure an update isn't in progress

    u8 second = get_from_cmos(CMOS_SECONDS);
    u8 minute = get_from_cmos(CMOS_MINUTES);
    u8 hour = get_from_cmos(CMOS_HOURS);
    u8 day = get_from_cmos(CMOS_MONTHDAY);
    u8 month = get_from_cmos(CMOS_MONTH);
    u16 year = get_from_cmos(CMOS_YEAR);
    u8 century = 0;
    if constexpr (CMOS_CENTURY != 0)
    {
        century = get_from_cmos(CMOS_CENTURY);
    }
    const u8 registerB = get_from_cmos(CMOS_STATUS_B);

    if (!(registerB & 0x04))
    {
        second = (second & 0x0F) + ((second / 16) * 10);
        minute = (minute & 0x0F) + ((minute / 16) * 10);
        hour = ((hour & 0x0F) + (((hour & 0x70) / 16) * 10)) | (hour & 0x80);
        day = (day & 0x0F) + ((day / 16) * 10);
        month = (month & 0x0F) + ((month / 16) * 10);
        year = (year & 0x0F) + ((year / 16) * 10);
        if (century != 0)
        {
            century = (century & 0x0F) + ((century / 16) * 10);
        }
    }

    // Check 12 hour
    if (!(registerB & 0x02) && (hour & 0x80)) {
        hour = ((hour & 0x7F) + 12) % 24;
    }

    // Calculate the full (4-digit) year accounting for change in century
    if constexpr (CMOS_CENTURY != 0) {
        year += century * 100;
    } else {
        year += (CURRENT_YEAR / 100) * 100;
        if(year < CURRENT_YEAR) year += 100;
    }

    current_time = {
        second, minute, hour, day, month, year, registerB
    };
    char outstr[20
        ];
    get_RTC_string(outstr);
    log.log("RTC read. New time: ", outstr);
}
