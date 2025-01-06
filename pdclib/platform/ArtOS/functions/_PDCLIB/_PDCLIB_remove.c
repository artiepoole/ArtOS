/* _PDCLIB_remove( const char * )

   This file is part of the Public Domain C Library (PDCLib).
   Permission is granted to use, modify, and / or redistribute at will.
*/

/* This is an example implementation of _PDCLIB_remove() fit for use with
   POSIX kernels.
*/

#ifndef REGTEST

#include "_PDCLIB_glue.h"
#include "stdio.h"
#include "stdlib.h"

#ifdef __cplusplus
extern "C" {
#endif

int unlink(const char* path)
{
    // TODO not implemented
    printf("ERROR: Unlink %s: not implemented", path);
    exit(1);
}
#ifdef __cplusplus
}
#endif

int _PDCLIB_remove(const char* pathname)
{
    return unlink(pathname);
}

#endif

#ifdef TEST

#include "_PDCLIB_test.h"

int main( void )
{
    /* Testing covered by ftell.c (and several others) */
    return TEST_RESULTS;
}

#endif
