//
// Created by artypoole on 22/11/24.
//

#ifndef SHELL_H
#define SHELL_H


class EventQueue;
class Terminal;
#include "types.h"

constexpr size_t cmd_buffer_size = 1024;

class Shell
{
public:
    explicit Shell(EventQueue* e);

    void run();

private:
    int process_cmd();
    EventQueue* events;
    u8 keyboard_modifiers = 0; // caps, ctrl, alt, shift  -> C ! ^ *
    size_t cmd_buffer_idx = 0;
    char cmd_buffer[cmd_buffer_size];
};


#endif //SHELL_H
