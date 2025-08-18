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
// Created by artypoole on 25/11/24.
//

#include "types.h"
#include "GDT.h"
#include "logging.h"
#include "string.h"

//TODO: incorrect size. The packing might be the issue. This should just be manual.
struct TSS_t
{
    u16 LINK;
    u16 res0;
    u32 ESP0;
    u16 SS0;
    u16 res1;
    u32 ESP1;
    u16 SS1;
    u16 res2;
    u32 ESP2;
    u16 SS2;
    u16 res3;
    u32 CR3, EIP, EFLAGS, EAX, ECX, EDX, EBX, ESP, EBP, ESI, EDI;
    u16 ES;
    u16 res4;
    u16 CS;
    u16 res5;
    u16 SS;
    u16 res6;
    u16 DS;
    u16 res7;
    u16 FS;
    u16 res8;
    u16 GS;
    u16 res9;
    u16 LDTR;
    u16 res10;
    u16 res11;
    u16 IOPB;
    u32 SSP; // shadow stack pointer
}__attribute__((packed));


union gdt_flags_t
{
    struct
    {
        u8 res : 1;
        u8 long_mode : 1; // set to 1 if 64-bit
        u8 double_mode : 1; // set to 1 if 32-bit and not 16-bit
        u8 paging_mode : 1; // set to 0 for limit is in bytes and 1 for limit is in 4kb pages.
    };

    u8 raw : 4;
};

union gdt_access_t
{
    struct
    {
        u8 accessed : 1;
        u8 RW : 1; // set to 1 if 64-bit
        u8 direction : 1; // for data set means segment grows down. For code set means can be executed by equal or lower priv
        u8 code : 1; // set to 1 for executable code and 0 for data segment
        u8 is_tss : 1;
        u8 privaledge_level : 2; // 0 for kernel only and 3 for user only
        u8 present : 1;
    };

    u8 raw;
};

struct gdtr_t
{
    u16 size;
    u32 offset;
}__attribute__((packed));

union raw_gdt_entry_t
{
    struct
    {
        u64 limit_low : 16;
        u64 base_low : 24;
        u64 access : 8;
        u64 limit_high : 4;
        u64 flags : 4;
        u64 base_high : 8;
    }__attribute__((packed));

    u64 raw;
};

struct nice_gdt_entry_t
{
    u32 base;
    u32 limit;
    gdt_access_t access;
    gdt_flags_t flags;
};

constexpr size_t n_entries = 6; // null, null, kernel cs, kernel ds, user cs, user ds, one tss

raw_gdt_entry_t ArtOS_GDT[n_entries];
gdtr_t ArtOS_GDTR;
TSS_t ArtOS_TSS0 = {};

const u32 bases[n_entries] = {0, 0, 0, 0, 0, reinterpret_cast<u32>(&ArtOS_TSS0)};
constexpr u32 limits[n_entries] = {0, 0xFFFFF, 0xFFFFF, 0xFFFFF, 0xFFFFF, sizeof(TSS_t) - 1};
constexpr u8 accesses[n_entries] = {0, 0x9a, 0x93, 0xFa, 0xF2, 0x89}; // TODO: explain these bits.
constexpr u8 flags[n_entries] = {0, 0xc, 0xc, 0xc, 0xc, 0x0}; // 0xc is double and paging modes
// extern void* kernel_stack_top;
extern void* kernel_interrupt_stack_top;
// TODO(CRITICAL): add a TSS for kernel mode.
// raw_gdt_entry_t* MB_GDT;

gdtr_t get_GDTR()
{
    gdtr_t gdt{};
    asm("sgdt %0" : "=m"(gdt));
    return gdt;
}

nice_gdt_entry_t ugly_to_nice(const raw_gdt_entry_t& ugly_entry)
{
    gdt_access_t access = {.raw = static_cast<u8>(ugly_entry.access)};
    const gdt_flags_t flags = {.raw = static_cast<u8>(ugly_entry.flags)};
    const nice_gdt_entry_t nice{
        .base = static_cast<u32>(ugly_entry.base_low | ugly_entry.base_high << 24),
        .limit = static_cast<u32>(ugly_entry.limit_low | ugly_entry.limit_high << 16),
        .access = access,
        .flags = flags,
    };
    return nice;
}

raw_gdt_entry_t nice_to_ugly(const nice_gdt_entry_t& nice_entry)
{
    const raw_gdt_entry_t ugly_entry{
        .limit_low = nice_entry.limit & 0xFFFF,
        .base_low = nice_entry.base & 0xFFFFFF, // 24bits
        .access = static_cast<u64>(nice_entry.access.raw & 0xFF),
        .limit_high = nice_entry.limit >> 16 & 0xF,
        .flags = static_cast<u64>(nice_entry.flags.raw & 0xF),
        .base_high = static_cast<u8>(nice_entry.base >> 24),
    };
    return ugly_entry;
}

u16 get_segment_offset(const size_t idx)
{
    return idx << 3; // ignore first 2 bits and multiply by 4
}

// Loads gdt and sets segment selectors appropriately. Implemented in GDT.S
extern "C"
void load_gdt(uintptr_t gdt_ptr, unsigned int data_sel, unsigned int code_sel);


void GDT_init()
{
    memset(&ArtOS_TSS0, 0, sizeof(TSS_t));
    ArtOS_TSS0.SS0 = kernel_ds_offset;
    ArtOS_TSS0.ESP0 = reinterpret_cast<uintptr_t>(&kernel_interrupt_stack_top);
    ArtOS_TSS0.IOPB = sizeof(TSS_t);

    for (size_t i = 0; i < n_entries; i++)
    {
        const gdt_flags_t flag = {.raw = flags[i]};
        const gdt_access_t access = {.raw = accesses[i]};
        ArtOS_GDT[i] = nice_to_ugly(
            nice_gdt_entry_t{
                .base = bases[i],
                .limit = limits[i],
                .access = access,
                .flags = flag
            }
        );
    }
    ArtOS_GDTR.offset = reinterpret_cast<u32>(&ArtOS_GDT);
    ArtOS_GDTR.size = n_entries * sizeof(raw_gdt_entry_t) - 1;

    // set_GDTR(ArtOS_GDTR.offset);
    uintptr_t gdtr_loc = reinterpret_cast<uintptr_t>(&ArtOS_GDTR);
    load_gdt(gdtr_loc, kernel_ds_offset, kernel_cs_offset);

    // TODO: This hardcoded 0x28 refers to the offset in the GDT that points to the TSS descriptor.
    u16 ltr = tss_offset;
    __asm__ __volatile(
        // "movw %0, %%ax\n\t"
        "ltr %0\n\t" // load the TSS.
        ::"r"(ltr)
    );
    // TODO: reload segments requiring long jump?
    LOG("GDT initialised.");
}
