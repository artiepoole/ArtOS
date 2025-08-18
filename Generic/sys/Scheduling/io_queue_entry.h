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

#ifndef IO_QUEUE_ENTRY_H
#define IO_QUEUE_ENTRY_H
#include <_types.h>

struct cpu_registers_t;

enum WaitingReason_t
{
    NOT_WAITING,
    FILE_READING,
    DEV_BUSY,
    FILE_WRITE,
};


class IO_operation
{
public:
    enum IO_State
    {
        NOT_STARTED,
        READY,
        IN_PROGRESS,
        DONE,
        ERROR,
    };

    virtual ~IO_operation() = default;
    virtual IO_State state() = 0;
    virtual void do_op() = 0;
    int* result;
    IO_State _state;
};

class IO_read final : public IO_operation
{
public:
    IO_read(int* r, int fd, char* dest, size_t count);
    IO_State state() override;
    void do_op() override;

private:
    int _fd;
    char* _buf;
    char* _k_buf;
    size_t _count;
};

struct io_queue_entry_t
{
    size_t process_id = 0;
    IO_operation* op = nullptr;
};


#endif //IO_QUEUE_ENTRY_H
