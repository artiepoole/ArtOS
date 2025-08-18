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

/* memset( void *, int, size_t )

   This file is part of the Public Domain C Library (PDCLib).
   Permission is granted to use, modify, and / or redistribute at will.
*/

#if SIMD
#include "SIMD.h"
#endif

#include "string.h"

#ifndef REGTEST


void* memset(void* s, int c, size_t n)
{
#if SIMD
    return simd_set(s, c, n);
#else
    unsigned char* p = (unsigned char*)s;

    while (n--)
    {
        *p++ = (unsigned char)c;
    }
#endif
    return s;
}

#endif

#ifdef TEST

#include "_PDCLIB_test.h"

int main( void )
{
    char s[] = "xxxxxxxxx";
    TESTCASE( memset( s, 'o', 10 ) == s );
    TESTCASE( s[9] == 'o' );
    TESTCASE( memset( s, '_', ( 0 ) ) == s );
    TESTCASE( s[0] == 'o' );
    TESTCASE( memset( s, '_', 1 ) == s );
    TESTCASE( s[0] == '_' );
    TESTCASE( s[1] == 'o' );
    return TEST_RESULTS;
}

#endif
