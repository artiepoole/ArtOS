//
// Created by artypoole on 16/08/24.
//

#include "APIC.h"

#include "ACPI.h"
#include "IDT.h"
#include "system.h"


#define IA32_APIC_BASE_MSR 0x1B
#define IA32_APIC_BASE_MSR_ENABLE 0x800


uintptr_t get_local_apic_base_addr()
{
    u32 eax;
    u32 edx;
    cpu_get_MSR(IA32_APIC_BASE_MSR, &eax, &edx);
    return (static_cast<uintptr_t>(eax) & 0xfffff000);
}

void set_local_apic_base_addr(const uintptr_t local_apic_base_addr)
{
    const u32 edx = 0;
    const u32 eax = (local_apic_base_addr & 0xfffff0000) | IA32_APIC_BASE_MSR_ENABLE;
    cpu_set_MSR(IA32_APIC_BASE_MSR, eax, edx);
}

