//
// Created by artypoole on 18/08/24.
//

#include "LocalAPIC.h"

uintptr_t* eoi_addr;

// LVT entry offsets
#define SPURIOUS_OFFSET_VECTOR 0xF0
#define TIMER_LVT_OFFSET 0x320
#define THERMAL_LVT_OFFSET 0x330
#define PERFORMANCE_LVT_OFFSET 0x340
#define LINT0_LVT_OFFSET 0x350
#define LINT1_LVT_OFFSET 0x360
#define ERROR_LVT_OFFSET 0x370

// other local apic offsets
#define TIMER_INITIAL_COUNT 0x380
#define TIMER_DIVISOR_OFFSET 0x3E0
#define LOCAL_APIC_ID_OFFSET 0x20
#define EOI_OFFSET 0xB0

LocalAPIC::LocalAPIC(uintptr_t local_apic_physical_address)
{
    base = reinterpret_cast<uintptr_t*>(local_apic_physical_address);
    eoi_addr = reinterpret_cast<uintptr_t*>(local_apic_physical_address + EOI_OFFSET);
    spurious = *reinterpret_cast<LVT_spurious_vector*>(local_apic_physical_address + SPURIOUS_OFFSET_VECTOR);

    // todo: troubleshoot not being able to set the value of the spurious vector here.
    full_lvt.timer = *reinterpret_cast<LVT_timer_entry*>(local_apic_physical_address + TIMER_LVT_OFFSET);
    full_lvt.thermal = *reinterpret_cast<LVT_entry*>(local_apic_physical_address + THERMAL_LVT_OFFSET);
    full_lvt.performance = *reinterpret_cast<LVT_entry*>(local_apic_physical_address + PERFORMANCE_LVT_OFFSET);
    full_lvt.LINT0 = *reinterpret_cast<LVT_entry*>(local_apic_physical_address + LINT0_LVT_OFFSET);
    full_lvt.LINT1 = *reinterpret_cast<LVT_entry*>(local_apic_physical_address + PERFORMANCE_LVT_OFFSET);
    full_lvt.error = *reinterpret_cast<LVT_entry*>(local_apic_physical_address + ERROR_LVT_OFFSET);
}

void LocalAPIC::configure_timer(const u32 hz)
{
    //TODO: implement https://github.com/dreamportdev/Osdev-Notes/blob/master/02_Architecture/08_Timers.md
}

void LAPIC_EOI()
{
    eoi_addr[0] = 0;
}