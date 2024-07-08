//
// Created by artypoole on 06/07/24.
//

#ifndef KEYBOARD_H
#define KEYBOARD_H


#include "ports.h"
#include "serial.h"
#include "vga.h"
extern char* keyboard_buffer;
extern size_t keyboard_buffer_size;


void keyboard_handler();

class Input{
}



#endif //KEYBOARD_H
