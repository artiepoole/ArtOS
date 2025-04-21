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
// Created by artiepoole on 4/21/25.
//

#ifndef PAGING_H
#define PAGING_H
#include "types.h"
#include "multiboot2.h"

constexpr size_t page_alignment = 4096;
constexpr size_t base_address_shift = 12;


union page_directory_4kb_t
{
    struct
    {
        u32 present : 1 = true;
        u32 rw : 1; // read/write if set, read-only otherwise
        u32 user_access : 1; // user and supervisor if set, supervisor only if not.
        u32 write_through : 1 = false; // write through caching if set, write-back otherwise
        u32 cache_disable : 1 = false; // is caching disabled?
        u32 accessed : 1 = false; // Gets sets on access, has to be cleared manually by OS if used.
        u32 OS : 1 = false; // free bit available for OS use
        u32 extended_page_size : 1 = false; // 4MB pages if set, 4KB if not. (Always not set in ArtOS).
        u32 OS_data : 4 = 0; // free nibble available for OS flags
        u32 page_table_entry_address : 20; // bits 12-31. 4KiB alignment means first 12 bits are always zero.
    };

    u32 raw;
};

union virtual_address_t
{
    uintptr_t raw;

    struct
    {
        uintptr_t page_offset : 12;
        uintptr_t page_table_index : 10;
        uintptr_t page_directory_index : 10;
    };
};

union page_table_entry_t
{
    struct
    {
        u32 present : 1 = true;
        u32 rw : 1; // read write if set, read only otherwise
        u32 user_access : 1; // user and supervisor if set, supervisor only if not.
        u32 write_through : 1 = false; // write through caching if set, write-back otherwise
        u32 cache_disable : 1 = false; // is caching disabled?
        u32 accessed : 1 = false; // Gets sets on access, has to be cleared manually by OS if used.
        u32 dirty : 1 = false; // "Has been written to if set"
        u32 page_attribute_table : 1 = false; // Res and zero probably: "If PAT is supported, then PAT along with PCD and PWT shall indicate the memory caching type. Otherwise, it is reserved and must be set to 0."
        u32 global : 1 = false; // "If PAT is supported, then PAT along with PCD and PWT shall indicate the memory caching type. Otherwise, it is reserved and must be set to 0."
        u32 OS_data : 3 = 0; // free nibble available for OS flags
        u32 physical_address : 20; // bits 12-31. 4KiB alignment means first 12 bits are always zero.
    };

    u32 raw;
};

page_table_entry_t paging_check_contents(uintptr_t vaddr);

void mmap_init(multiboot2_tag_mmap* mmap);
//
void* kmmap(uintptr_t addr, size_t length, int prot, int flags, int fd, size_t offset);
int kmunmap(void* addr, size_t length);
//
void paging_identity_map(uintptr_t phys_addr, size_t size, bool writable, bool user);
// uintptr_t paging_get_phys_addr(uintptr_t vaddr);
void paging_set_target_pid(size_t pid);

extern unsigned char* kernel_brk;

#endif //PAGING_H
