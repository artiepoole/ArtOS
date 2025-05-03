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
// Created by artypoole on 20/08/24.
//

#ifndef LOGGING_H
#define LOGGING_H

#pragma once
#if ENABLE_SERIAL_LOGGING && ENABLE_TERMINAL_LOGGING
    #include "Terminal.h"
    #include "Serial.h"
    #define LOG(...) Terminal::get().log(__VA_ARGS__); get_serial().log(__VA_ARGS__);
    #define WRITE(...) Terminal::get().write(__VA_ARGS__); get_serial().write(__VA_ARGS__)
    #define NEWLINE() Terminal::get().newLine(); get_serial().newLine()
    #define TIMESTAMP() Terminal::get().time_stamp(); get_serial().time_stamp()
#elif ENABLE_TERMINAL_LOGGING
    #include "Terminal.h"
    #define LOG(...) Terminal::get().log(__VA_ARGS__)
    #define WRITE(...) Terminal::get().write(__VA_ARGS__)
    #define NEWLINE() Terminal::get().newLine()
    #define TIMESTAMP() Terminal::get().time_stamp()
#elif ENABLE_SERIAL_LOGGING
    #include "Serial.h"
    #define LOG(...) get_serial().log(__VA_ARGS__)
    #define WRITE(...) get_serial().write(__VA_ARGS__)
    #define NEWLINE() get_serial().newLine()
    #define TIMESTAMP() get_serial().time_stamp()
#else
    #define LOG(...)
    #define WRITE(...)
    #define NEWLINE()
    #define TIMESTAMP()
#endif


#endif
