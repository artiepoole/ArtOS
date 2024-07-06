//
// Created by artypoole on 26/06/24.
//

#include "string.h"

#include "serial.h"



size_t strlen(const char* str)
{
    int len = 0;
    while (str[len])
        len++;
    return len;
}




