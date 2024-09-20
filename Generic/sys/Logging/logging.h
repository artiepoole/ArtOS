//
// Created by artypoole on 20/08/24.
//

#ifndef LOGGING_H
#define LOGGING_H

#pragma once
#if ENABLE_SERIAL_LOGGING && ENABLE_TERMINAL_LOGGING
    #include "Terminal.h"
    #include "Serial.h"
    #define LOG(...) Terminal::get().log(__VA_ARGS__); Serial::get().log(__VA_ARGS__);
    #define WRITE(...) Terminal::get().write(__VA_ARGS__); Serial::get().write(__VA_ARGS__)
    #define NEWLINE() Terminal::get().newLine(); Serial::get().newLine()
    #define TIMESTAMP() Terminal::get().time_stamp(); Serial::get().time_stamp()
#elif ENABLE_TERMINAL_LOGGING
    #include "Terminal.h"
    #define LOG(...) Terminal::get().log(__VA_ARGS__)
    #define WRITE(...) Terminal::get().write(__VA_ARGS__)
    #define NEWLINE() Terminal::get().newLine()
    #define TIMESTAMP() Terminal::get().time_stamp()
#elif ENABLE_SERIAL_LOGGING
    #include "Serial.h"
    #define LOG(...) Serial::get().log(__VA_ARGS__)
    #define WRITE(...) Serial::get().write(__VA_ARGS__)
    #define NEWLINE() Serial::get().newLine()
    #define TIMESTAMP() Serial::get().time_stamp()
#else
    #define LOG(...)
    #define WRITE(...)
    #define NEWLINE()
    #define TIMESTAMP()
#endif


#endif
