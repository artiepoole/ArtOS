//
// Created by artypoole on 26/06/24.
//

#include "mystring.h"


// Get length of string including first termination character.
size_t mystrlen(const char* str)
{
    int len = 0;
    while (str[len])
        len++;
    return len+1;
}




