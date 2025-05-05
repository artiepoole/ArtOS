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
// Created by artiepoole on 5/3/25.
//

#ifndef BART_H
#define BART_H

#include "kernel.h"

constexpr size_t cmd_buffer_size = 128;

class BartShell
{
public:
    BartShell();
    void run();

private:
    int process_cmd();
    u8 keyboard_modifiers = 0; // caps, ctrl, alt, shift  -> C ! ^ *
    size_t cmd_buffer_idx = 0;
    char cmd_buffer[cmd_buffer_size];
};


int main();



#endif //BART_H
