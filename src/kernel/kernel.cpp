//
// Created by artypoole on 09/07/24.
//

#include "kernel.h"

#include "Terminal.h"

#include "../include/VideoGraphicsArray.h"

void write_standard(const char* buffer, size_t len)
{
    auto& term = Terminal::get();
    term.writeBuffer(buffer, len);

    auto& log = Serial::get();
    log.writeBuffer(buffer, len);

}

void write_error(char* buffer, size_t len)
{
    // todo: implement the propagation of colour so that this can be overridden to use red for errors or something.
    auto& term = Terminal::get();
    term.writeBuffer(buffer, len);

    auto& log = Serial::get();
    log.writeBuffer(buffer, len);
}
