//
// Created by artypoole on 22/11/24.
//

#ifndef SHELL_H
#define SHELL_H

#include "types.h"

class EventQueue;
class Terminal;

constexpr size_t cmd_buffer_size = 128;

class Shell
{
public:
    Shell();
    void run();

private:
    int process_cmd();
    u8 keyboard_modifiers = 0; // caps, ctrl, alt, shift  -> C ! ^ *
    size_t cmd_buffer_idx = 0;
    char cmd_buffer[cmd_buffer_size];
};


inline void shell_run()
{
    // Init and load the shell. Shell draws directly to the terminal by using static methods.
    Shell shell;
    shell.run();
}


#endif //SHELL_H
