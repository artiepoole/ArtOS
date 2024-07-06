//
// Created by artypoole on 26/06/24.
//

#include "string.h"

#include "serial.h"



int strlen(const char* str)
{
    int len = 0;
    while (str[len])
        len++;
    return len;
}




