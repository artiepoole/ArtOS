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

#include "syscall.h"

#include <Files.h>
#include "memory.h"
#include "paging.h"

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
void _kexit(size_t target_pid)
{
    Scheduler::kill(target_pid);
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
    // TODO: pit is disabled so should replace with RTC or similar.
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
    }
}

void ksleep_us(const u32 us)
{
    const u32 start = kget_tick_us();
    while (kget_tick_us() - start < us)
    {
    }
}


void kpause_exec(const u32 ms)
{
    PIT_sleep_ms(ms);
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

#include "../../ArtOS_lib/kernel.h"

void syscall_handler(cpu_registers_t* r)
{
    u64 large_res;
    switch (static_cast<SYSCALL_t>(r->eax))
    {
    case SYSCALL_t::WRITE:
        // TODO: this is no where near this simple for hardware files. Interrupts are needed for IO so the task must be slept and the interrupt handled correctly.
        // TODO: mapping user space data!
        // TODO: mapping a single page will be a problem if the buffer is close to the end of the page

        r->eax = art_write(static_cast<int>(r->ebx), reinterpret_cast<char*>(r->ecx), r->edx);
        break;
    case SYSCALL_t::READ:
        // TODO: this is no where near this simple for hardware files. Interrupts are needed for IO so the task must be slept and the interrupt handled correctly.
        // TODO: mapping user space data!

        r->eax = art_read(static_cast<int>(r->ebx), reinterpret_cast<char*>(r->ecx), r->edx);
        break;
    case SYSCALL_t::OPEN:
        // TODO: this is no where near this simple for hardware files. Interrupts are needed for IO so the task must be slept and the interrupt handled correctly.
        // TODO: mapping user space data!
        art_open(reinterpret_cast<char*>(r->ebx), r->ecx);
        break;
    case SYSCALL_t::CLOSE:
        art_close(r->ebx);
        break;
    case SYSCALL_t::EXIT:
        Scheduler::exit(r);
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
    case SYSCALL_t::MMAP:
        // *reinterpret_cast<u32*>(r->ebp) + 7)
        r->eax = reinterpret_cast<u32>(kmmap(r->ebx, r->ecx, r->edx, r->esi, r->edi, 0));
        break;
    case SYSCALL_t::MUNMAP:
        kmunmap(reinterpret_cast<void*>(r->ebx), r->ecx);
        break;
    case SYSCALL_t::GET_CURRENT_CLOCK:
        large_res = kget_current_clock();
        r->eax = static_cast<u32>(large_res);
        r->ebx = static_cast<u32>(large_res >> 32);
        break;
    case SYSCALL_t::EXECF:
        Scheduler::execf(r, r->ebx, r->ecx, r->edx);
        break;
    case SYSCALL_t::YIELD:
        Scheduler::schedule(r);
        break;
    default:
        LOG("Unhandled Syscall: ", static_cast<u32>(r->eax));
        r->eax = -1;
        break;
    }

    // TODO:
    // write read open and seek syscalls should call those functions from Files.h
}
