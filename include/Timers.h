//
// Created by artypoole on 13/07/24.
//

#ifndef TIME_H
#define TIME_H



#include "Serial.h"
#include <float.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

// rtc to be used for system clock time ticking and pit to be used for scheduling.

struct RTC_t // using u16 for ease of printing.
{
    u8 second, minute, hour, day, month, weekday;
    u16 year; //  4 digit year
    u8 regB;
};

void get_RTC_string(char* out_str);

void configurePit(u32 hz);

void sleep(u32 ms);

void timerHandler();

void RTCHandler();

#endif //TIME_H
