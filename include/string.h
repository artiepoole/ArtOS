//
// Created by artypoole on 26/06/24.
//

#ifndef STRING_H
#define STRING_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include "types.h"

static constexpr char hex[16] = {'0','1','2','3','4','5','6','7','8','9','A','B','C','D','E','F'};

extern "C"
int strlen(const char* str);

extern "C"
int string_from_int(u64 val, char* out_str);

extern "C"
int hex_from_int(u64 val, char* out_str, u32 n_bytes);

#endif //STRING_H
