//
// Created by artypoole on 26/06/24.
//

#ifndef STRING_H
#define STRING_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

size_t strlen(const char* str);

size_t string_from_int(long val, char* out_str);



#endif //STRING_H
