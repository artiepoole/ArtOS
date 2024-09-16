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
    return RTC::get().getTime();
}


time_t get_epoch_time()
{
    return RTC::get().epochTime();
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

uint32_t get_tick_s()
{
    return TSC_get_ticks()/(SMBIOS_get_CPU_clock_rate_hz());
}

uint32_t get_tick_ms()
{
    return TSC_get_ticks()/(SMBIOS_get_CPU_clock_rate_hz()/1000);
}

uint32_t get_tick_us()
{
    return TSC_get_ticks()/(SMBIOS_get_CPU_clock_rate_mhz());
}
uint32_t get_tick_ns()
{
    return TSC_get_ticks()/(SMBIOS_get_CPU_clock_rate_mhz()/1000);
}

void sleep_s(const u32 s)
{
    const u32 ms = s*1000;
    // if s*1000 out of bounds for u32 then handle that by sleeping for s lots of milliseconds a thousand times.
    if (ms < s){ for (u32 i = 0; i < 1000; i++) PIT_sleep_ms(s);}
    PIT_sleep_ms(ms);
}

void sleep_ms(const u32 ms)
{
    PIT_sleep_ms(ms);
}

void sleep_ns(const u32 ns)
{
    const u32 start = get_tick_ns();
    while (get_tick_ns() - start < ns){};
}

void sleep_us(const u32 us)
{
    const u32 start = get_tick_us();
    while (get_tick_us() - start < us){};
}

void draw_screen_region(const u32* frame_buffer)
{
    VideoGraphicsArray::get().draw_region(frame_buffer);
}
