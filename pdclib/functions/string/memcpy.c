/* memcpy( void *, const void *, size_t )

   This file is part of the Public Domain C Library (PDCLib).
   Permission is granted to use, modify, and / or redistribute at will.
*/

#include "string.h"

#ifndef REGTEST


#include "SIMD.h"


void* memcpy(void* _PDCLIB_restrict dest, const void* _PDCLIB_restrict src, size_t n)
{
    if (simd_enabled())
    {
        simd_copy(dest, src, n);
    }
    else
    {
        char* s1 = (char*)dest;
        const char* s2 = (const char*)src;

        while (n--)
        {
            *s1++ = *s2++;
        }
    }

    return dest;
}

#endif

#ifdef TEST

#include "_PDCLIB_test.h"

int main( void )
{
    char s[] = "xxxxxxxxxxx";
    TESTCASE( memcpy( s, abcde, 6 ) == s );
    TESTCASE( s[4] == 'e' );
    TESTCASE( s[5] == '\0' );
    TESTCASE( memcpy( s + 5, abcde, 5 ) == s + 5 );
    TESTCASE( s[9] == 'e' );
    TESTCASE( s[10] == 'x' );
    return TEST_RESULTS;
}

#endif
