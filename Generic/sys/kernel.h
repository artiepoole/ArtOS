//
// Created by artypoole on 09/07/24.
//

#ifndef KERNEL_H
#define KERNEL_H

#include "time.h"
#include "types.h"
#include "event.h"


enum class SYSCALL_t
{
    WRITE,
    READ,
    OPEN,
    EXIT,
    SLEEP_MS,
    GET_TICK_MS,
    PROBE_EVENTS,
    GET_EVENT,
    DRAW_REGION,
    GET_TIME,
    GET_EPOCH
};


// files
int write(int fd, const char* buf, unsigned long count);
int read(int fd, char* buf, size_t count);
int open(const char* pathname, int flags);
void _exit(int status);

// time
void sleep_ms(u32 ms);
uint32_t get_tick_ms();
int get_time(tm* dest);
time_t get_epoch_time();

// events
bool probe_pending_events();
event_t get_next_event();

//graphics
void draw_screen_region(const u32* frame_buffer);

#endif //KERNEL_H
