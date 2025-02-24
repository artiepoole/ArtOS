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

#include "CPU.h"
#include "CPUID.h"
#include "logging.h"


eflags_t get_eflags()
{
    eflags_t flags;
    asm volatile("pushf ; pop %0" : "=rm" (flags)::"memory");

    return flags;
}

bool get_interrupts_are_enabled()
{
    return get_eflags().IF;
}


void disable_interrupts()
{
    __asm__ volatile ("cli"); // clear the interrupt flag
}

void enable_interrupts()
{
    __asm__ volatile ("sti"); // set the interrupt flag
}


void write_register(uintptr_t addr, u32 val)
{
    *(volatile uintptr_t*)addr = val;
}

u32 read_register(uintptr_t addr)
{
    return *reinterpret_cast<uintptr_t*>(addr);
}

void cpu_get_MSR(u32 msr, u32* lo, u32* hi)
{
    asm volatile("rdmsr" : "=a"(*lo), "=d"(*hi) : "c"(msr));
}

void cpu_set_MSR(u32 msr, u32 lo, u32 hi)
{
    asm volatile("wrmsr" : : "a"(lo), "d"(hi), "c"(msr));
}


u16 get_cs()
{
    u16 cs;
    asm("mov %%cs,%0" : "=r"(cs));
    return cs;
}

u16 get_ds()
{
    u16 ds;
    asm("mov %%ds,%0" : "=r"(ds));
    return ds;
}


cr0_t get_cr0()
{
    cr0_t cr0{};
    asm("mov %%cr0,%0" : "=r"(cr0.raw));
    return cr0;
}

u32 get_cr2()
{
    u32 cr2{};
    asm("mov %%cr2,%0" : "=r"(cr2));
    return cr2;
}
