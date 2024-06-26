//
// Created by artypoole on 26/06/24.
//

#ifndef STRING_H
#define STRING_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

extern "C"
size_t strlen(const char* str);

extern "C"
size_t string_from_int(long val, char* out_str);


#endif //STRING_H
