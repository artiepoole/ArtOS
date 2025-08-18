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
// Created by artypoole on 18/07/24.
//


#include "PIT.h"

#include <IOAPIC.h>

#include "logging.h"
#include "ports.h"

u32 rate = 0;

volatile u32 timer_ticks = 0;
IOAPIC *ioapic;
size_t myirq;

#define PIT_CONTROL_PORT 0x43
#define PIT_CHANNEL0_PORT 0x40
#define PIT_BASE_FREQUENCY 1193182  // Hz

void configure_pit(const u32 hz, IOAPIC *ioapic_, size_t irq) {
    // In square wave mode, the PIT double counts.
    myirq = irq;
    ioapic = ioapic_;
    LOG("Initialising PIT");
    const u32 divisor = PIT_BASE_FREQUENCY / (2 * hz); /* Calculate our divisor */
    rate = 2 * PIT_BASE_FREQUENCY / divisor; // calculating back to get the real rate after integer maths
    LOG("\tConfigured PIT. Divisor: ", divisor, " rate: ", rate);
    // 0x36 = 00 11 011 0
    // 00 = Channel 0
    // 11 = Access mode: lobyte/hibyte
    // 011 = Mode 3 (square wave)
    // 0 = Binary counting mode
    outb(PIT_CONTROL_PORT, 0x36);
    outb(PIT_CHANNEL0_PORT, divisor & 0xFF);
    outb(PIT_CHANNEL0_PORT, (divisor >> 8) & 0xFF);

    LOG("PIT initialised");
}


void PIT_sleep_ms(const u32 ms) {
    if (rate == 0) {
        LOG("Tried to PIT_sleep_ms when timer is not initiated.");
        return;
    }
    timer_ticks = static_cast<uint64_t>(ms) * rate / 1000;
    ioapic->enable_IRQ(myirq);
    while (timer_ticks > 0) {
        asm volatile("hlt;");
    }
    ioapic->disable_IRQ(myirq);
}

void pit_handler() {
    // Check if PIT_sleep_ms is still active.
    if (timer_ticks == 0) return;
    timer_ticks -= 1;
}
