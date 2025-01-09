//
// Created by artypoole on 09/01/25.
//

#include "kernel.h"
#include "event.h"


extern "C" int write(int fd, const char* buf, unsigned long count)
{
    int result;
    asm volatile(
        "int $50" // Trigger software interrupt
        : "=a"(result)
        : "a"(SYSCALL_t::WRITE), "b"(fd), "c"(buf), "d"(count)
        : "memory"
    );
    return result;
}

extern "C" int read(int fd, char* buf, size_t count)
{
    int result;
    asm volatile(
        "int $50" // Trigger software interrupt
        : "=a"(result)
        : "a"(SYSCALL_t::READ), "b"(fd), "c"(buf), "d"(count)
        : "memory"
    );
    return result;
}

int open(const char* pathname, int flags)
{
    int result;
    asm volatile(
        "int $50" // Trigger software interrupt
        : "=a"(result)
        : "a"(SYSCALL_t::OPEN), "b"(pathname), "c"(flags)
        : "memory"
    );
    return result;
}

void _exit(int status)
{
    asm volatile(
        "int $50" // Trigger software interrupt
        :
        : "a"(SYSCALL_t::EXIT), "b"(status)
        : "memory"
    );
}

void sleep_ms(u32 ms)
{
    asm volatile(
        "int $50" // Trigger software interrupt
        :
        : "a"(SYSCALL_t::SLEEP_MS), "b"(ms)
        : "memory"
    );
}

uint32_t get_tick_ms()
{
    uint32_t result;
    asm volatile(
        "int $50" // Trigger software interrupt
        : "=a"(result)
        : "a"(SYSCALL_t::GET_TICK_MS)
        : "memory"
    );
    return result;
}

bool probe_pending_events()
{
    bool result;
    asm volatile(
        "int $50" // Trigger software interrupt
        : "=a"(result)
        : "a"(SYSCALL_t::PROBE_EVENTS)
        : "memory"
    );
    return result;
}

event_t get_next_event()
{
    u32 type, data_low, data_high;
    asm volatile(
        "int $50" // Trigger software interrupt
        : "=a"(type), "=b"(data_low), "=c"(data_high)
        : "a"(SYSCALL_t::GET_EVENT)
        : "memory"
    );
    return event_t{static_cast<EVENT_TYPE>(type), event_data_t{data_low, data_high}};
}

void draw_screen_region(const u32* frame_buffer)
{
    asm volatile(
        "int $50" // Trigger software interrupt
        :
        : "a"(SYSCALL_t::DRAW_REGION), "b"(frame_buffer)
        : "memory"
    );
}

int get_time(tm* dest)
{
    int result;
    asm volatile(
        "int $50" // Trigger software interrupt
        : "=a"(result)
        : "a"(SYSCALL_t::GET_TIME), "b"(dest)
        : "memory"
    );
    return result;
}

long get_epoch_time()
{
    long result;
    asm volatile(
        "int $50" // Trigger software interrupt
        : "=a"(result)
        : "a"(SYSCALL_t::GET_EPOCH)
        : "memory"
    );
    return result;
}
