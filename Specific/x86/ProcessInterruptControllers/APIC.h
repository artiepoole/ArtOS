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

#ifndef APIC_H
#define APIC_H
#include "types.h"


union LVT_timer_entry
{
    struct
    {
        u32 interrupt_vector : 8;
        u32 pad0 : 4;
        u32 delivery_status : 1; // 12
        u32 pad1 : 3;
        u32 interrupt_mask : 1;
        u32 timer_mode : 2;
        u32 pad : 13;
    } parts;

    u32 raw;
}__attribute__((packed));


union LVT_entry
{
    struct
    {
        u32 interrupt_vector : 8;
        u32 delivery_mode : 3;
        u32 destination_mode : 1;
        u32 delivery_status : 1;
        u32 pin_polarity : 1;
        u32 remote_IRR : 1;
        u32 trigger_mode : 1;
        u32 interrupt_mask : 1;
        u32 pad : 15;
    };

    u32 entry;
}__attribute__((packed));


union LVT_spurious_vector
{
    struct
    {
        u32 spurious_vector : 8;
        u32 software_enable : 1;
        u32 focus_checking : 1;
        u32 reserved : 22;
    };

    u32 raw;
}__attribute__((packed));


struct LVT
{
    LVT_timer_entry timer;
    LVT_entry thermal;
    LVT_entry performance;
    LVT_entry LINT0;
    LVT_entry LINT1;
    LVT_entry error;
};


// Helper functions but are not used. Can be used to remap the APIC
uintptr_t get_local_apic_base_addr();
void set_local_apic_base_addr(uintptr_t addr);


#endif //APIC_H
