/* memcpy( void *, const void *, size_t )

   This file is part of the Public Domain C Library (PDCLib).
   Permission is granted to use, modify, and / or redistribute at will.
*/

#include "string.h"

#ifndef REGTEST

/*
 * TODO: add compiler flag for SIMD or on the fly detection support
 */
#define SIMD_SUPPORT

#ifndef SIMD_SUPPORT
#else
#include "SIMD.h"
#endif


void * memcpy( void * _PDCLIB_restrict s1, const void * _PDCLIB_restrict s2, size_t n )
{
#ifndef SIMD_SUPPORT
    char * dest = ( char * ) s1;
    const char * src = ( const char * ) s2;

    while ( n-- )
    {
        *dest++ = *src++;
    }
#else
    simd_copy(s1, s2, n);
#endif
    return s1;
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
