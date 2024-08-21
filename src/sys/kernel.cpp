//
// Created by artypoole on 09/07/24.
//

#include "kernel.h"
#include "logging.h"
#include "PIT.h"
#include "VideoGraphicsArray.h"
#include "TSC.h"
#include "SMBIOS.h"
#include "Serial.h"

// u64 clock_rate = 0;

void write_standard(const char* buffer, unsigned long len)
{
    // auto& term = Terminal::get();
    // term.write(buffer, len);


    WRITE(buffer, len);
}

void write_error(const char* buffer, unsigned long len)
{
    // // todo: implement the propagation of colour so that this can be overridden to use red for errors or something.
    // auto& term = Terminal::get();
    // term.write(buffer, len, COLOR_RED);


    WRITE(buffer, len);
}

tm* get_time()
{
    auto& rtc = RTC::get();
    return rtc.getTime();
}


time_t get_epoch_time()
{
    auto& rtc = RTC::get();
    return rtc.epochTime();
}

extern "C"
void _exit(int status)
{

    LOG("Exit status: ", status);
}

u64 get_clock_rate_hz()
{

    return SMBIOS_get_CPU_clock_rate_hz();;
}

u64 get_current_clock()
{
    return TSC_get_ticks();
}

uint32_t get_tick_ms()
{
    return TSC_get_ticks()/(SMBIOS_get_CPU_clock_rate_hz()/1000);
}

void sleep_ms(const u32 ms)
{
    sleep(ms);
}

void draw_screen_region(const u32* frame_buffer)
{
    VideoGraphicsArray::get().draw_region(frame_buffer);
}
