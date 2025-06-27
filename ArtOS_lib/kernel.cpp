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
// Created by artypoole on 09/01/25.
//

#include "kernel.h"
#include "event.h"
#include "keymaps/key_maps.h"


extern "C" {
int write(int fd, const char *buf, unsigned long count) {
    int result;
    asm volatile(
        "int $0x80" // Trigger software interrupt
        : "=a"(result)
        : "a"(SYSCALL_t::WRITE), "b"(fd), "c"(buf), "d"(count)
        : "memory"
    );
    return result;
}

int read(int fd, char *buf, size_t count) {
    int result;
    asm volatile(
        "int $0x80" // Trigger software interrupt
        : "=a"(result)
        : "a"(SYSCALL_t::READ), "b"(fd), "c"(buf), "d"(count)
        : "memory"
    );
    return result;
}

int open(const char *pathname, int flags) {
    int result;
    asm volatile(
        "int $0x80" // Trigger software interrupt
        : "=a"(result)
        : "a"(SYSCALL_t::OPEN), "b"(pathname), "c"(flags)
        : "memory"
    );
    return result;
}

void _exit(int status) {
    asm volatile(
        "int $0x80" // Trigger software interrupt
        :
        : "a"(SYSCALL_t::EXIT), "b"(status)
        : "memory"
    );
    while (true);
}

void sleep_ms(u32 ms) {
    asm volatile(
        "int $0x80" // Trigger software interrupt
        :
        : "a"(SYSCALL_t::SLEEP_MS), "b"(ms)
        : "memory"
    );
}

u32 get_tick_ms() {
    u32 result;
    asm volatile(
        "int $0x80" // Trigger software interrupt
        : "=a"(result)
        : "a"(SYSCALL_t::GET_TICK_MS)
        : "memory"
    );
    return result;
}

bool probe_pending_events() {
    bool result;
    asm volatile(
        "int $0x80" // Trigger software interrupt
        : "=a"(result)
        : "a"(SYSCALL_t::PROBE_EVENTS)
        : "memory"
    );
    return result;
}

event_t get_next_event() {
    u32 type, data_low, data_high;
    asm volatile(
        "int $0x80" // Trigger software interrupt
        : "=a"(type), "=b"(data_low), "=c"(data_high)
        : "a"(SYSCALL_t::GET_EVENT)
        : "memory"
    );
    return event_t{static_cast<EVENT_TYPE>(type), event_data_t{data_low, data_high}};
}

void draw_screen_region(const u32 *frame_buffer) {
    asm volatile(
        "int $0x80" // Trigger software interrupt
        :
        : "a"(SYSCALL_t::DRAW_REGION), "b"(frame_buffer)
        : "memory"
    );
}

int get_time(tm *dest) {
    int result;
    asm volatile(
        "int $0x80" // Trigger software interrupt
        : "=a"(result)
        : "a"(SYSCALL_t::GET_TIME), "b"(dest)
        : "memory"
    );
    return result;
}

long get_epoch_time() {
    long result;
    asm volatile(
        "int $0x80" // Trigger software interrupt
        : "=a"(result)
        : "a"(SYSCALL_t::GET_EPOCH)
        : "memory"
    );
    return result;
}

u32 get_ebp() {
    u32 ebp;
    asm("mov %%ebp,%0" : "=r"(ebp));
    return ebp;
}

// u32 set_ebp(u32 ebp)
// {
//     asm volatile("mov %0, %%ebp" :: "r"(ebp));
// }

void *mmap(void *addr, size_t length, int prot, int flags, int fd, size_t offset) {
    // u32 old_ebp;

    void *result;
    // asm volatile( "push %0"::"r"(offset):"memory");
    asm volatile(
        "int $0x80" // Trigger software interrupt
        : "=a"(result)
        : "a"(SYSCALL_t::MMAP), // syscall ID
        "b"(addr), // arg1 ebx
        "c"(length), // arg2 ecx
        "d"(prot), // arg3 edx
        "S"(flags), // arg4 esi
        "D"(fd) // arg5 edi
        : "memory"
    );
    // set_ebp(old_ebp); // restore ebp
    // asm volatile("mov %0, %%ebp" :: "r"(old_ebp));
    return result;
}

void munmap(void *addr, size_t length) {
    asm volatile(
        "int $0x80" // Trigger software interrupt
        :
        : "a"(SYSCALL_t::MMAP), // syscall ID
        "b"(addr),
        "c"(length)
        : "memory"
    );
}

u64 get_current_clock() {
    int low, high;
    asm volatile(
        "int $0x80" // Trigger software interrupt
        : "=a"(low), "=b"(high)
        : "a"(SYSCALL_t::GET_CURRENT_CLOCK)
        : "memory"
    );
    return low | (static_cast<u64>(high) << 32);
}

int execf(int fd) {
    int ret;
    asm volatile(
        "int $0x80" // Trigger software interrupt
        :"=a"(ret)
        : "a"(SYSCALL_t::EXECF), "b"(fd)
        : "memory"
    );
    return ret;
}

void yield() {
    asm volatile(
        "int $0x80" // Trigger software interrupt
        :
        : "a"(SYSCALL_t::YIELD) // syscall ID
        : "memory"
    );
}
}

int close(const int fd) {
    int ret;
    asm volatile(
        "int $0x80" // Trigger software interrupt
        :"=a"(ret)
        : "a"(SYSCALL_t::CLOSE), "b"(fd)
        : "memory"
    );
    return ret;
}

int seek(int fd, i64 offset, int whence) {
    int ret;
    u32 off_low = offset & 0xFFFFFFFF;
    i32 off_high = offset >> 32;
    asm volatile(
        "int $0x80" // Trigger software interrupt
        :"=a"(ret)
        : "a"(SYSCALL_t::SEEK), "b"(fd), "c"(off_high), "d"(off_low), "S"(whence)
        : "memory"
    );
    return ret;
}
