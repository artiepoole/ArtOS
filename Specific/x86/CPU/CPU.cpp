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
