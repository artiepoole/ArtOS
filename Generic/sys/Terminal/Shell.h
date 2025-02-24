// ArtOS - hobby operating system by Artie Poole
// Copyright (C) 2025 Stuart Forbes Poole <artiepoole>
//
//     This program is free software: you can redistribute it and/or modify
//     it under the terms of the GNU General Public License as published by
//     the Free Software Foundation, either version 3 of the License, or
//     (at your option) any later version.
//
//     This program is distributed in the hope that it will be useful,
//     but WITHOUT ANY WARRANTY; without even the implied warranty of
//     MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//     GNU General Public License for more details.
//
//     You should have received a copy of the GNU General Public License
//     along with this program.  If not, see <https://www.gnu.org/licenses/>

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
