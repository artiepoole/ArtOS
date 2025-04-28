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

#include "PagingTableKernel.h"

#include <memory.h>
#include <string.h>

#include "logging.h"

// constexpr size_t max_n_pages = 0x100000;

/// Each table is 4k in size, and is page aligned i.e. 4k aligned. They consists of 1024 32 bit entries.
constexpr size_t page_table_size = 1024;

// bitmaps used to keep track of the next virtual and physical pages available.
// These are the same during identity mapping, but diverge when user space programs make malloc calls
// 4GB worth of 4096 pages. Set all to false. Processing the multiboot2 memory_map will set the necessary bits.
// constexpr size_t n_DBs = (max_n_pages + (64 - 1)) / 32;
// u64 paging_phys_bitmap_array[n_DBs];
// u64 paging_virt_bitmap_array[n_DBs];
// bool Scheduler::isProcessUser(size_t PID)
// {
//     return processes[PID].user;
// }

PagingTableKernel::PagingTableKernel(multiboot2_tag_mmap* mmap)
{
    paging_table = reinterpret_cast<page_directory_4kb_t**>(art_alloc(sizeof(page_directory_4kb_t) * page_table_size), page_alignment);
    memset(paging_table, 0, page_table_size * sizeof(page_directory_4kb_t));
    n_tables = 0;
    n_entries = 0;
    // PagingTableKernel::append_page_table(true);
}

uintptr_t PagingTableKernel::get_page_table_addr()
{
    return reinterpret_cast<uintptr_t>(paging_table);
}

page_table_entry_t* PagingTableKernel::append_page_table(const bool writable)
{
    // auto new_table_entry = art_alloc();
    memset(new_table_entry, 0, sizeof(page_table_entry_t));
    size_t target_idx = 0;
    while (target_idx < 1024 & paging_table[target_idx]->present)
    {
        target_idx++;
    }
    if (target_idx >= 1024)
    {
        // TODO: THIS IS OUT OF MEMORY
        for (;;);
    }

    auto dir_entry = paging_table[target_idx];
    dir_entry->page_table_entry_address = reinterpret_cast<u32>(new_table_entry) >> base_address_shift;
    dir_entry->rw = writable;
    dir_entry->user_access = false;
    dir_entry->present = true;
}
