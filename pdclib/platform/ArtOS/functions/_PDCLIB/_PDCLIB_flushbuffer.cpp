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

/* _PDCLIB_flushbuffer( struct _PDCLIB_file_t * )

   This file is part of the Public Domain C Library (PDCLib).
   Permission is granted to use, modify, and / or redistribute at will.
*/

/* This is an example implementation of _PDCLIB_flushbuffer() fit for
   use with POSIX kernels.
*/

#include <Files.h>

#include "stdio.h"
#include "string.h"

#ifndef REGTEST

#include <kernel.h>

#include "_PDCLIB_glue.h"
#include "errno.h"

// #ifdef __cplusplus
// extern "C" {
// #endif

typedef long ssize_t;

// extern int write(int fd, const char* buf, unsigned long count);

// #ifdef __cplusplus
// }
// #endif

/* The number of attempts of output buffer flushing before giving up.         */
#define _PDCLIB_IO_RETRIES 1

/* What the system should do after an I/O operation did not succeed, before   */
/* trying again. (Empty by default.)                                          */
#define _PDCLIB_IO_RETRY_OP( stream )

int _PDCLIB_flushbuffer(struct _PDCLIB_file_t* stream)
{
    /* No need to handle buffers > INT_MAX, as PDCLib doesn't allow them */
    _PDCLIB_size_t written = 0;
    unsigned int retries;

    if (!(stream->status & _PDCLIB_FBIN))
    {
        /* TODO: Text stream conversion here */
    }

    /* Keep trying to write data until everything is written, an error
       occurs, or the configured number of retries is exceeded.
    */
    for (retries = _PDCLIB_IO_RETRIES; retries > 0; --retries)
    {
        int rc = write(stream->handle, stream->buffer + written, stream->bufidx - written);

        if (rc < 0)
        {
            /* The 1:1 mapping done in _PDCLIB_config.h ensures
               this works.
            */
            *_PDCLIB_errno_func() = errno;
            /* Flag the stream */
            stream->status |= _PDCLIB_ERRORFLAG;
            /* Move unwritten remains to begin of buffer. */
            stream->bufidx -= written;
            memmove(stream->buffer, stream->buffer + written, stream->bufidx);
            return EOF;
        }

        written += (_PDCLIB_size_t)rc;
        stream->pos.offset += rc;

        if (written == stream->bufidx)
        {
            /* Buffer written completely. */
            stream->bufidx = 0;
            return 0;
        }
    }

    /* Number of retries exceeded. */
    *_PDCLIB_errno_func() = _PDCLIB_EAGAIN;
    stream->status |= _PDCLIB_ERRORFLAG;
    /* Move unwritten remains to begin of buffer. */
    stream->bufidx -= written;
    memmove(stream->buffer, stream->buffer + written, stream->bufidx);
    return EOF;
}

#endif

#ifdef TEST

#include "_PDCLIB_test.h"

int main( void )
{
    /* Testing covered by ftell.c */
    return TEST_RESULTS;
}

#endif
