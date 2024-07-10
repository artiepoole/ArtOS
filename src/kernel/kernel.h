//
// Created by artypoole on 09/07/24.
//

#ifndef KERNEL_H
#define KERNEL_H

#include <stddef.h>


void write_standard(const char* buffer, size_t len);

void write_error(const char* buffer, size_t len);

void draw_screen_region(unsigned long * frame_buffer);


#endif //KERNEL_H
