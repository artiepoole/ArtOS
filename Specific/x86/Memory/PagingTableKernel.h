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

#include "PagingTable.h"

class PagingTableKernel : PagingTable
{
public:
    explicit PagingTableKernel(multiboot2_tag_mmap* mmap);
    uintptr_t get_page_table_addr() override;
    page_table_entry_t* append_page_table(bool writable) override;
    void direct_map(uintptr_t sector_start, size_t sector_size, u8 permissions);
    // void* mmap(uintptr_t addr, size_t length, int prot, int flags, int fd, size_t offset);
    // int munmap(void* addr, size_t length);
private:
    page_directory_4kb_t** paging_table = nullptr; // stores 1024 page tables
    size_t n_tables = 0;
    size_t n_entries = 0;

    size_t get_assign_vaddr();
    // TODO: The paging table is ALWAYS stored in kernel memory.
    // TODO: the structure is: a single paging_dir stores the locations of 1024 paging tables. We only ever need one of these and it has to be contiguous
    // Each paging table is 1024 paging_table_entries and this is what we need to make sure we create when we fill one
    // Each page's physical addr is stored in one of these paging table entries.
    // If not doing a direct map (kernel typically doesn't direct map after init) then we just need to make a way for the kernel to get the available virtual addresses.
    // For this, I think it's quickest to use a virtual_address bitmap only for the kernel, and to traverse the paging_dir->paging_table->entry to see if it is present.
    // If it is not, then we malloc that and mark it as present and apply the appropriate flags.
    // For user space, we will just loop through the tables stored in the dict to find one that isn't present
    // and then step back and check if the last entry is present on prev table. If so, go to the next directory_entry and malloc a table into it.
};


#endif //PAGINGTABLEKERNEL_H
