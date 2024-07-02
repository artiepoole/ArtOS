//
// Created by artypoole on 25/06/24.
//
#ifndef SERIAL_H
#define SERIAL_H


#include "string.h"

#include "ports.h"

#define PORT 0x3f8          // COM1

void write_serial(char a);

void serial_write_string(const char* data);

void serial_new_line();

void serial_write_int(long val);

void serial_write_hex(long val);

extern "C"
int serial_initialise();






#endif //SERIAL_H
