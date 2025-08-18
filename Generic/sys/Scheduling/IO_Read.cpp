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

//
// Created by artiepoole on 7/6/25.
//
#include <CPU.h>
#include <cstdio>
#include <Files.h>
#include <paging.h>
#include <PagingTableKernel.h>
#include <Process.h>

#include "io_queue_entry.h"

IO_read::IO_read(int* r, const int fd, char* dest, const size_t count)
{
    _fd = fd;
    _buf = dest;
    const size_t offset_in_page = (reinterpret_cast<uintptr_t>(dest) % page_alignment);
    _k_buf = reinterpret_cast<char*>(kernel_pages().map_user_to_kernel(
        reinterpret_cast<uintptr_t>(_buf),
        count + offset_in_page) + offset_in_page);
    _count = count;
    _state = NOT_STARTED;
    result = r;
}

IO_operation::IO_State IO_read::state()
{
    if (_state == NOT_STARTED && !art_dev_busy(_fd))
    {
        _state = READY;
    }
    if (_state == IN_PROGRESS)
    {
        if (const i64 n_read = art_async_n_read(_fd); n_read == _count)
        {
            const size_t offset_in_page = (reinterpret_cast<uintptr_t>(_k_buf) % page_alignment);
            kernel_pages().unmap_user_to_kernel(reinterpret_cast<uintptr_t>(_k_buf),
                                                _count + offset_in_page);
            art_seek(_fd, n_read, SEEK_CUR);
            *result = static_cast<int>(n_read);
            _state = DONE;
        }
    }
    return _state;
}

void IO_read::do_op()
{
#if ASYNC_READ
    switch (const int res = art_async_read(_fd, _k_buf,
                                           _count))
    {
    case -1:
#if ENABLE_SERIAL_LOGGING and LOG_SYSCALL
        get_serial().log("Async not enabled, using synchronous read");
#endif
        *result = art_read(_fd, _k_buf, _count);
        _state = DONE;
        break;
    case 0:
#if ENABLE_SERIAL_LOGGING and LOG_SYSCALL
        get_serial().log("async read started");
#endif
        _state = IN_PROGRESS;
        break;
    default:
#if ENABLE_SERIAL_LOGGING and LOG_SYSCALL
        get_serial().log(res, " bytes of data already in buffer, returning directly");
#endif
        *result = res;
        // If immediate, must seek else the seek happens on complete read.
        art_seek(_fd, res, SEEK_CUR);
        _state = DONE;
        break;
    }
#else
#if ENABLE_SERIAL_LOGGING and LOG_SYSCALL
    get_serial().log("Async not enabled, using synchronous read");
#endif
    *ret = art_read(_fd, _k_buf, _count);
#endif
}
