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

#ifndef IOAPIC_H
#define IOAPIC_H

#include "types.h"
#include "APIC.h"


union io_redirect_entry
{
    struct
    {
        LVT_entry lvt;
        u32 reserved : 24;
        u32 destination_lapic_addr : 4;
        u32 destination_cluster_addr : 4;
    };

    struct
    {
        u32 lower;
        u32 upper;
    };
};


class IOAPIC
{
public:
    IOAPIC(uintptr_t io_apic_physical_address);
    void pause();
    void resume();
    void remap_IRQ(u8 irq_before, u8 irq_after);
    void disable_IRQ(u8 irq_before);
    void enable_IRQ(u8 irq_before);
    void enable_all();

private:
    uintptr_t volatile* base_addr;
    uintptr_t volatile* data_addr;
    io_redirect_entry redirect_entries[23];
};


#endif //IOAPIC_H
