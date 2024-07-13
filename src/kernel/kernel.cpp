//
// Created by artypoole on 09/07/24.
//

#include "kernel.h"

#include "../include/VideoGraphicsArray.h"

void write_standard(const char* buffer, const size_t len)
{
    auto& term = Terminal::get();
    term.write(buffer, len);

    auto& log = Serial::get();
    log.writeBuffer(buffer, len);
}

void write_error(char* buffer, const size_t len)
{
    // todo: implement the propagation of colour so that this can be overridden to use red for errors or something.
    auto& term = Terminal::get();
    term.write(buffer, len, COLOR_RED);

    auto& log = Serial::get();
    log.writeBuffer(buffer, len);
}

// void draw_screen_region(u32* frame_buffer)
// {
//     auto &vga = VideoGraphicsArray::get();
//     vga.
// }
