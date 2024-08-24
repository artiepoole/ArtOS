#include "CPU.h"

// #include "Serial.h"
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
    LOG("\tInterrupts disabled");
    __asm__ volatile ("cli"); // clear the interrupt flag
}

void enable_interrupts()
{
    LOG("\tInterrupts enabled");
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


struct gdt_info
{
    u16 limit;
    u32 base;
}__attribute__((packed));

struct gdt_entry
{
    u16 limit_low;
    u16 base_low;
    u8 base_middle;
    u8 access;
    u8 granularity;
    u8 base_high;
} __attribute__((packed));


void get_GDT()
{
    gdt_info gdt{};
    asm("sgdt %0" : "=m"(gdt));
    WRITE("GDT limit: ");
    WRITE(gdt.limit, true);
    WRITE(" GDT base: ");
    WRITE(gdt.base, true);
    NEWLINE();

    for (size_t i = 0; i < 8; i++)
    {
        WRITE("GDT entry:");
        WRITE(i);
        WRITE(" data: ");
        [[maybe_unused]] uintptr_t gdt_ptr = static_cast<ptrdiff_t>(gdt.base + (8 * i));
        WRITE(gdt_ptr, true);
        NEWLINE();
    }
}

u16 get_cs()
{
    u16 i;
    asm("mov %%cs,%0" : "=r"(i));
    return i;
}

u16 get_ds()
{
    u16 i;
    asm("mov %%ds,%0" : "=r"(i));
    return i;
}