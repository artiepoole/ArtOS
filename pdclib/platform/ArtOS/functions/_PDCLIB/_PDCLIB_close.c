/* _PDCLIB_close( _PDCLIB_fd_t )

   This file is part of the Public Domain C Library (PDCLib).
   Permission is granted to use, modify, and / or redistribute at will.
*/

/* This is an example implementation of _PDCLIB_close() fit for use with POSIX
   kernels.
*/

#include "stdio.h"
#include "stdlib.h"

#ifndef REGTEST

#include "_PDCLIB_glue.h"

#ifdef __cplusplus
extern "C" {
#endif
extern int close(size_t fd);

#ifdef __cplusplus
}
#endif

int _PDCLIB_close(int fd)
{
    return close(fd);
}

#endif

#ifdef TEST

#include "_PDCLIB_test.h"

int main( void )
{
    /* No testdriver; tested in driver for _PDCLIB_open(). */
    return TEST_RESULTS;
}

#endif
