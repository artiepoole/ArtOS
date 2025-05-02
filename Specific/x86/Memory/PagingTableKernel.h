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

#ifndef PAGINGTABLEKERNEL_H
#define PAGINGTABLEKERNEL_H

#include <DenseBooleanArray.h>

#include "PagingTable.h"
#include "paging.h"


class PagingTableKernel : public PagingTable
{
public:
    PagingTableKernel() = default;
    void late_init();
    void assign_page_directory_entry(size_t dir_idx, bool writable, bool user);
    void* mmap(uintptr_t addr, size_t length, int prot, int flags, int fd, size_t offset) override;
    void paging_identity_map(uintptr_t phys_addr, size_t size, bool writable, bool user);
    int munmap(void* addr, size_t length_bytes) override;
    uintptr_t get_page_table_addr() override;
    uintptr_t page_get_next_virtual_chunk(size_t idx, size_t n_pages);
    uintptr_t get_next_virtual_addr(uintptr_t start_addr);
    bool dir_entry_present(size_t idx) override;
    uintptr_t get_phys_from_virtual(uintptr_t v_addr) override;
    page_table_entry_t check_vmap_contents(uintptr_t v_addr) override;
    void assign_page_table_entry(uintptr_t physical_addr, virtual_address_t v_addr, bool writable, bool user) override;
    int unassign_page_table_entries(size_t start_idx, size_t n_pages) override;
    void direct_map(uintptr_t sector_start, size_t sector_size, u8 permissions);

};


// TODO: you can mark all tables as present and identity map all of memory for the kernel and instead keep track of allocated memory another way if you want.
// This takesthe smae amount of space and time as doing it how I do it but allocating new pages for use in kernel is quick.
// TODO: The paging table is ALWAYS stored in kernel memory.
// TODO: the structure is: a single paging_dir stores the locations of 1024 paging tables. We only ever need one of these and it has to be contiguous
// Each paging table is 1024 paging_table_entries and this is what we need to make sure we create when we fill one
// Each page's physical addr is stored in one of these paging table entries.
// If not doing a direct map (kernel typically doesn't direct map after init) then we just need to make a way for the kernel to get the available virtual addresses.
// For this, I think it's quickest to use a virtual_address bitmap only for the kernel, and to traverse the paging_dir->paging_table->entry to see if it is present.
// If it is not, then we malloc that and mark it as present and apply the appropriate flags.
// For user space, we will just loop through the tables stored in the dict to find one that isn't present
// and then step back and check if the last entry is present on prev table. If so, go to the next directory_entry and malloc a table into it.

#endif //PAGINGTABLEKERNEL_H
