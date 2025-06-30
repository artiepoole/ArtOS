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
// Created by artypoole on 09/07/24.
//

#ifndef KERNEL_H
#define KERNEL_H

#include "_types.h"


#ifdef __cplusplus
class ELF;

extern "C" {
#endif


enum SYSCALL_t {
    WRITE,
    READ,
    SEEK,
    OPEN,
    CLOSE,
    EXIT,
    SLEEP_MS,
    GET_TICK_MS,
    GET_CURRENT_CLOCK,
    PROBE_EVENTS,
    GET_EVENT,
    DRAW_REGION,
    CLEAR_TERM,
    GET_TIME,
    GET_EPOCH,
    MMAP,
    MUNMAP,
    EXECF,
    YIELD
};

typedef struct tm tm;
typedef struct event_t event_t;

// files
int write(int fd, const char *buf, unsigned long count);

int read(int fd, char *buf, size_t count);

i64 seek(int fd, i64 offset, int whence);

int open(const char *pathname, int flags);

int close(const int fd);

void _exit(int status);

// time
void sleep_ms(u32 ms);

u32 get_tick_ms();

int get_time(tm *dest);

long get_epoch_time();

u64 get_current_clock();

// events
bool probe_pending_events();

event_t get_next_event();

// graphics
void draw_screen_region(const u32 *frame_buffer);

void clear_term();

// memory
void *mmap(void *addr, size_t length, int prot, int flags, int fd, size_t offset);

void munmap(void *addr, size_t length);

// scheduling
int execf(int fid);

void yield();
#ifdef __cplusplus
}
#endif
#endif //KERNEL_H
