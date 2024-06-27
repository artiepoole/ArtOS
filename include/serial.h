//
// Created by artypoole on 25/06/24.
//
#ifndef SERIAL_H
#define SERIAL_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#define PORT 0x3f8          // COM1

void write_serial(char a);

void serial_write_string(const char* data);

void serial_new_line();

extern "C"
int serial_initialise();



#endif //SERIAL_H
