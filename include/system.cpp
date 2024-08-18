#include <system.h>

#include "Serial.h"

eflags_t get_eflags()
{
    eflags_t flags;
    asm volatile("pushf ; pop %0" : "=rm" (flags)::"memory");

    return flags;
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
