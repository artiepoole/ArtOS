//
// Created by artypoole on 09/07/24.
//

#include "kernel.h"

#include "../include/VideoGraphicsArray.h"

void write_standard(const char* buffer, unsigned long len)
{
    auto& term = Terminal::get();
    term.write(buffer, len);

    auto& log = Serial::get();
    log.write(buffer, len);
}

void write_error(const char* buffer, unsigned long len)
{
    // todo: implement the propagation of colour so that this can be overridden to use red for errors or something.
    auto& term = Terminal::get();
    term.write(buffer, len, COLOR_RED);

    auto& log = Serial::get();
    log.write(buffer, len);
}

tm get_time()
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
    auto & log = Serial::get();
    log.log("Exit status: ",status);
}

// void draw_screen_region(u32* frame_buffer)
// {
//     auto &vga = VideoGraphicsArray::get();
//     vga.
// }
