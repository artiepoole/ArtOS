//
// Created by artypoole on 09/07/24.
//

#ifndef SYSCALL_H
#define SYSCALL_H

#include "event.h"

#include "types.h"
#include "Terminal.h"
#include "RTC.h"

enum class SYSCALL_t
{
    WRITE,
    READ,
    OPEN,
    CLOSE,
    EXIT,
    SLEEP_MS,
    GET_TICK_MS,
    PROBE_EVENTS,
    GET_EVENT,
    DRAW_REGION,
    GET_TIME,
    GET_EPOCH
};

void kwrite_standard(const char* buffer, unsigned long len);

void kwrite_error(const char* buffer, unsigned long len);


template <typename type_t>
void kprint(type_t const& arg1)
{
    auto& term = Terminal::get();
    term.write(arg1);
    term.newLine();
}

template <typename... args_t>
void kprint(args_t&&... args)
{
    auto& term = Terminal::get();
    (term.write(args), ...);
    term.newLine();
}

template <typename... args_t>
void kprint_colour(u32 color, args_t&&... args)
{
    auto& term = Terminal::get();
    (term.write(args, color), ...);
    term.newLine();
}

template <typename int_like>
    requires is_int_like_v<int_like> && (!is_same_v<int_like, char>)
void kprint_hex(int_like val, size_t hex_len, u32 color = COLOR_BASE00)
{
    auto& term = Terminal::get();
    term.write(val, hex_len, color);
    term.newLine();
}

extern "C" {
void kexit(size_t target_pid);

int kget_time(tm* dest);
time_t kget_epoch_time();

u64 kget_clock_rate_hz();
u64 kget_current_clock();
uint32_t kget_tick_s();
uint32_t kget_tick_ms();
uint32_t kget_tick_us();
uint32_t kget_tick_ns();

void ksleep_s(u32 s);
void ksleep_ms(u32 ms);
void ksleep_us(u32 us);
void ksleep_ns(u32 ns);
void kget_clock_ms();
void kpause_exec(const u32 ms);

bool kprobe_pending_events();
event_t kget_next_event();

void kdraw_screen_region(const u32* frame_buffer);

void syscall_handler(cpu_registers_t* r);
}

#endif //SYSCALL_H
