//
// Created by artypoole on 18/08/24.
//

#ifndef LAPIC_H
#define LAPIC_H

#include "types.h"
#include "APIC.h"

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

private:
    uintptr_t base;
    LVT full_lvt;
    LVT_spurious_vector volatile * spurious_vector_entry;
    u32 LAPIC_ratio = 0;
    u32 LAPIC_rate = 0;
};

void LAPIC_EOI();

void LAPIC_handler();
void LAPIC_calibrate_handler();


#endif //LAPIC_H
