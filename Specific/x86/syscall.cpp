//
// Created by artypoole on 09/07/24.
//

#include "syscall.h"

#include <Files.h>

#include "EventQueue.h"
#include "Scheduler.h"
#include "SMBIOS.h"

#include "logging.h"
#include "PIT.h"
#include "VideoGraphicsArray.h"
#include "TSC.h"
#include "RTC.h"

// u64 clock_rate = 0;

void kwrite_standard([[maybe_unused]] const char* buffer, [[maybe_unused]] unsigned long len)
{
    WRITE(buffer, len);
}

void kwrite_error([[maybe_unused]] const char* buffer, [[maybe_unused]] unsigned long len)
{
    // // todo: implement the propagation of colour so that this can be overridden to use red for errors or something.
    // auto& term = Terminal::get();
    // term.write(buffer, len, COLOR_RED);

    WRITE(buffer, len);
}

int kget_time(tm* mytm)
{
    return RTC::get().getTime(mytm);
}


time_t kget_epoch_time()
{
    return RTC::get().epochTime();
}

extern "C"
void _kexit([[maybe_unused]] int status)
{
    Scheduler::exit(status);
}

u64 kget_clock_rate_hz()
{
    return SMBIOS_get_CPU_clock_rate_hz();
}

u64 kget_current_clock()
{
    return TSC_get_ticks();
}

uint32_t kget_tick_s()
{
    return TSC_get_ticks() / (SMBIOS_get_CPU_clock_rate_hz());
}

uint32_t kget_tick_ms()
{
    return TSC_get_ticks() / (SMBIOS_get_CPU_clock_rate_hz() / 1000);
}

uint32_t kget_tick_us()
{
    return TSC_get_ticks() / (SMBIOS_get_CPU_clock_rate_mhz());
}

uint32_t kget_tick_ns()
{
    return TSC_get_ticks() / (SMBIOS_get_CPU_clock_rate_mhz() / 1000);
}

// TODO: replace PIT_sleep* with scheduler sleep
void ksleep_s(const u32 s)
{
    const u32 ms = s * 1000;
    // if s*1000 out of bounds for u32 then handle that by sleeping for s lots of milliseconds a thousand times.
    if (ms < s) { for (u32 i = 0; i < 1000; i++) PIT_sleep_ms(s); }
    PIT_sleep_ms(ms);
}

void ksleep_ms(const u32 ms)
{
    Scheduler::sleep_ms(ms);
    // PIT_sleep_ms(ms);
}

void ksleep_ns(const u32 ns)
{
    const u32 start = kget_tick_ns();
    while (kget_tick_ns() - start < ns)
    {
    };
}

void ksleep_us(const u32 us)
{
    const u32 start = kget_tick_us();
    while (kget_tick_us() - start < us)
    {
    };
}


void kpause_exec(const u32 ms)
{
    Scheduler::sleep_ms(ms);
}

bool kprobe_pending_events()
{
    return Scheduler::getCurrentProcessEventQueue()->pendingEvents();
}

event_t kget_next_event()
{
    // TODO: this should only get events associated with the correct event queue.
    // There should not be a generic system event queue but instead there should be one associated with each processes
    return Scheduler::getCurrentProcessEventQueue()->getEvent();
}

void kdraw_screen_region(const u32* frame_buffer)
{
    VideoGraphicsArray::get().drawRegion(frame_buffer);
}

#include "kernel.h"

void syscall_handler(cpu_registers_t* r)
{
    switch (static_cast<SYSCALL_t>(r->eax))
    {
    case SYSCALL_t::WRITE:
        art_write(static_cast<int>(r->ebx), reinterpret_cast<char*>(r->ecx), r->edx);
        break;
    case SYSCALL_t::READ:
        art_read(static_cast<int>(r->ebx), reinterpret_cast<char*>(r->ecx), r->edx);
        break;
    case SYSCALL_t::OPEN:
        art_open(reinterpret_cast<char*>(r->ebx), r->ecx);
        break;
    case SYSCALL_t::EXIT:
        Scheduler::kill(r);
        break;
    case SYSCALL_t::SLEEP_MS:
        ksleep_ms(r->ebx);
        break;
    case SYSCALL_t::GET_TICK_MS:
        r->eax = kget_tick_ms();
        break;
    case SYSCALL_t::PROBE_EVENTS:
        r->eax = kprobe_pending_events();
        break;
    case SYSCALL_t::GET_EVENT:
        {
            auto [type, data] = kget_next_event();
            r->eax = type;
            r->ebx = data.lower_data;
            r->ecx = data.upper_data;
            break;
        }
    case SYSCALL_t::DRAW_REGION:
        kdraw_screen_region(reinterpret_cast<u32*>(r->ebx));
        break;
    case SYSCALL_t::GET_TIME:
        r->eax = kget_time(reinterpret_cast<tm*>(r->ebx));
        break;
    case SYSCALL_t::GET_EPOCH:
        r->eax = kget_epoch_time();
        break;
    default:
        LOG("Unhandled Syscall.");
        r->eax = -1;
        break;
    }

    // TODO:
    // write read open and seek syscalls should call those functions from Files.h
}
