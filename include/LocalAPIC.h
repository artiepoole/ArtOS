//
// Created by artypoole on 18/08/24.
//

#ifndef LAPIC_H
#define LAPIC_H

#include "types.h"
#include "APIC.h"

class LocalAPIC
{
public:
    LocalAPIC(uintptr_t local_apic_physical_address);
    void configure_timer(u32 hz);

private:
    uintptr_t base;
    LVT full_lvt;
    LVT_spurious_vector volatile * spurious_vector_entry;
};

void LAPIC_EOI();





#endif //LAPIC_H
