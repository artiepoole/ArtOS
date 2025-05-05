// ArtOS - hobby operating system by Artie Poole
// Copyright (C) 2025 Stuart Forbes Poole <artiepoole>
//
//     This program is free software: you can redistribute it and/or modify
//     it under the terms of the GNU General Public License as published by
//     the Free Software Foundation, either version 3 of the License, or
//     (at your option) any later version.
//
//     This program is distributed in the hope that it will be useful,
//     but WITHOUT ANY WARRANTY; without even the implied warranty of
//     MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//     GNU General Public License for more details.
//
//     You should have received a copy of the GNU General Public License
//     along with this program.  If not, see <https://www.gnu.org/licenses/>

/* memmove( void *, const void *, size_t )

   This file is part of the Public Domain C Library (PDCLib).
   Permission is granted to use, modify, and / or redistribute at will.
*/

// #include "SIMD.h"
#include <string.h>

#ifndef REGTEST

void* memmove(void* s1, const void* s2, size_t n)
{
    // if (simd_enabled())
    // {
    //     return simd_move(s1, s2, n);
    // }
    char* dest = (char*)s1;
    const char* src = (const char*)s2;

    if (dest <= src)
    {
        while (n--)
        {
            *dest++ = *src++;
        }
    }
    else
    {
        src += n;
        dest += n;

        while (n--)
        {
            *--dest = *--src;
        }
    }

    return s1;
}

#endif

#ifdef TEST

#include "_PDCLIB_test.h"

int main( void )
{
    char s[] = "xxxxabcde";
    TESTCASE( memmove( s, s + 4, 5 ) == s );
    TESTCASE( s[0] == 'a' );
    TESTCASE( s[4] == 'e' );
    TESTCASE( s[5] == 'b' );
    TESTCASE( memmove( s + 4, s, 5 ) == s + 4 );
    TESTCASE( s[4] == 'a' );
    return TEST_RESULTS;
}

#endif
