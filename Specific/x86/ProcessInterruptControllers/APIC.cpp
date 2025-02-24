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
// Created by artypoole on 16/08/24.
//

#include "APIC.h"

#include "ACPI.h"
#include "CPU.h"


constexpr u32 IA32_APIC_BASE_MSR = 0x1B;
constexpr u32 IA32_APIC_BASE_MSR_ENABLE = 0x800;


uintptr_t get_local_apic_base_addr()
{
    u32 eax;
    u32 edx;
    cpu_get_MSR(IA32_APIC_BASE_MSR, &eax, &edx);
    return eax & 0xfffff000;
}

void set_local_apic_base_addr(const uintptr_t local_apic_base_addr)
{
    const u32 edx = 0;
    const u32 eax = (local_apic_base_addr & 0xfffff0000) | IA32_APIC_BASE_MSR_ENABLE;
    cpu_set_MSR(IA32_APIC_BASE_MSR, eax, edx);
}
