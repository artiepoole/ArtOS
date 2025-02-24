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
// Created by artypoole on 13/07/24.
//

#ifndef MEMORY_H
#define MEMORY_H


#include "types.h"

// http://wiki.osdev.org/Memory_Map_(x86)
// "Use the BIOS function INT 15h, EAX=0xE820 to get a reliable map of Extended Memory."
// http://wiki.osdev.org/Detecting_Memory_(x86)#BIOS_Function:_INT_0x15,_EAX_=_0xE820
#ifdef __cplusplus
extern "C" {
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

#endif

struct multiboot2_tag_mmap; // forward dec - multiboot2.h
extern unsigned char* kernel_brk; // TODO: needed by paging.cpp. Better practice?

// implemented in Specific/<arc>/memory/<paging or similar>.cpp
void mmap_init(struct multiboot2_tag_mmap* mmap);

void* mmap(uintptr_t addr, size_t length, int prot, int flags, int fd, size_t offset);
int munmap(void* addr, size_t length);

void paging_identity_map(uintptr_t phys_addr, size_t size, bool writable, bool user);
uintptr_t paging_get_phys_addr(uintptr_t vaddr);


void* aligned_malloc(size_t size, size_t alignment);

void aligned_free(void* ptr);


#ifdef __cplusplus
page_table_entry_t paging_check_contents(uintptr_t vaddr);
}
#endif
#endif //MEMORY_H
