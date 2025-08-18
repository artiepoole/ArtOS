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
// Created by artypoole on 19/07/24.
//

/* SMBIOS contains lots of system information generated during the boot. This emans that it stores things such as the cpu speed which,
 * in our case, cannot be detected using CPUID.
 * Useful links:
 *
 * https://wiki.osdev.org/System_Management_BIOS
 * https://www.dmtf.org/sites/default/files/standards/documents/DSP0134_3.2.0.pdf
 *
 */

#ifndef SMBIOS_H
#define SMBIOS_H

#include "types.h"



struct smbios_t_32
{
    u32 anchor_str;
    u8 check, length, major_version, minor_version;
    u16 max_size;
    u8 revision;
    u8 area_0, area_1, area_2, area_3, area_4, anchor_1, anchor_2, anchor_3, anchor_4, intermediate_checksum;
    u16 structure_table_length;
    u32 structure_table_address;
    u16 n_structures;
    u8 BDC_rev;
};

struct smbios_t_64
{
    u8 anchor_str[5];
    u8 check;
    u8 length;
    u8 major_revision;
    u8 minor_revision;
    u8 docrev;
    u8 entry_point_rev;
    u8 res0;
    u32 structure_table_max_size;
    u64 structure_table_address;
};

// Stub of an entry which describes what is in the entry and how long it is.
// Note that the length here does not include strings which occur after the data table and so the end of the strings
// must be searched for ("\0\0" terminated)
struct smbios_header_t
{
    u8 type, length;
    u16 handle;
};


struct smbios_processor_info_t
{
    u8 type, length;
    u16 handle; // First three items are the header
    u8 socket, processor_type, family1, manufacturer;
    u64 id;
    u8 version, voltage;
    u16 clock_freq, max_clock, current_speed;
    u8 status, upgrade;
    u16 L1_cache_handle, L2_cache_handle, L3_cache_handle;
    u8 serial_no, asset_tag, part_no, core_count_1, core_enabled_1, thread_count_1;
    u16 characteristics, family2, core_count_2, core_enabled_2, thread_count_2;
};


address_range search_for_SMBIOS();

void SMBIOS_populate_cpu_info();

u64 SMBIOS_get_CPU_clock_rate_hz();

u64 SMBIOS_get_CPU_clock_rate_mhz();


#endif //SMBIOS_H
