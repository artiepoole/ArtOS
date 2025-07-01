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
// Created by artypoole on 18/08/24.
//

#ifndef LAPIC_H
#define LAPIC_H

#include "types.h"
#include "APIC.h"
#include "CPU.h"

//Divide Value (bits 0, 1 and 3)
// 000: Divide by 2
// 001: Divide by 4
// 010: Divide by 8
// 011: Divide by 16
// 100: Divide by 32
// 101: Divide by 64
// 110: Divide by 128
// 111: Divide by 1

enum DIVISOR
{
    DIVISOR_1 = 0b1011,
    DIVISOR_2 = 0b0000,
    DIVISOR_4 = 0b0001,
    DIVISOR_8 = 0b0010,
    DIVISOR_16 = 0b0011,
    DIVISOR_32 = 0b1000,
    DIVISOR_64 = 0b1001,
    DIVISOR_128 = 0b1010
};

class LocalAPIC
{
public:
    LocalAPIC(uintptr_t local_apic_physical_address);
    void calibrate_timer();
    void configure_timer(DIVISOR divisor);
    bool ready() const;
    int start_timer_ms(u32 ms) const;
    int start_timer_us(u32 us) const;
private:
    uintptr_t base;
    LVT full_lvt;
    LVT_spurious_vector volatile* spurious_vector_entry;
    u32 LAPIC_rate = 0;
    bool is_ready = false;
};

void LAPIC_EOI();

extern void LAPIC_handler(cpu_registers_t* const r);
void LAPIC_calibrate_handler();


#endif //LAPIC_H
