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



struct smbios_t
{
    u32 anchor_str;
    u8 check, length, major_version, minor_version;
    u16 max_size;
    u8 revision;
    u8 area_0, area_1, area_2, area_3, area_4, anchor_1, anchor_2, anchor_3, anchor_4, intermediate_checksum;
    u16 stucture_table_length;
    u32 structure_table_address;
    u16 n_structures;
    u8 BDC_rev;
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


smbios_t search_for_SMBIOS();

void SMBIOS_populate_cpu_info();

i32 SMBIOS_get_CPU_clock_rate_hz();


#endif //SMBIOS_H
