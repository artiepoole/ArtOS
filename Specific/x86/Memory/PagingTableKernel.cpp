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
    PagingTableKernel::append_page_table();
    // TODO: create (virtual and ?) physical bitmaps
    // TODO: populate with the map like mmap init used to
    // page_available_virtual_bitmap.init(paging_virt_bitmap_array, max_n_pages, true);
    // page_available_physical_bitmap.init(paging_phys_bitmap_array, max_n_pages, false);
    //
    //
    // const auto brk_loc = reinterpret_cast<uintptr_t>(kernel_brk);
    // const size_t n_entries = mmap->size / sizeof(multiboot2_mmap_entry);
    // const uintptr_t post_kernel_page = ((brk_loc >> base_address_shift) + 1) << base_address_shift; // first page after kernel image.
    // uintptr_t last_end = 0;
    //
    // // Loop through all entries and map appropriately
    // for (size_t i = 0; i < n_entries; i++)
    // {
    //     multiboot2_mmap_entry const* entry = mmap->entries[i];
    //     if (entry->addr > last_end)
    //     {
    //         // fill holes
    //         paging_identity_map(last_end, entry->addr - last_end, true, false);
    //     }
    //
    //     if (entry->addr < brk_loc && entry->addr + entry->len > brk_loc)
    //     {
    //         // contains kernel
    //         // only map used kernel region.
    //         paging_identity_map(entry->addr, brk_loc - entry->addr, entry->type == 1 && entry->addr > 0, false);
    //         main_region_start = entry->addr;
    //         main_region_end = entry->addr + entry->len;
    //         last_physical_idx = main_region_end >> base_address_shift;
    //     }
    //     else
    //     {
    //         paging_identity_map(entry->addr, entry->len, entry->type == 1 && entry->addr > 0, false);
    //     }
    //
    //     last_end = entry->addr + entry->len;
    // }
    //
    // // Protect kernel and init identity map
    // // paging_identity_map(main_region_start, post_kernel_page - main_region_start, true, false);
    // paging_identity_map(0xf0000000, 0xffffffff - 0xf0000000, true, false);
    //
    // // set upper limit in physical bitmap to extents of the avaialble
    // page_available_physical_bitmap.set_range(
    //     post_kernel_page >> base_address_shift,
    //     (main_region_end - post_kernel_page) >> base_address_shift,
    //     true
    // );
    //
    //
    // LOG("Paging: memory map processed.");
    // enable_paging();
    // LOG("Paging: paging enabled.");
}

uintptr_t PagingTableKernel::get_page_table_addr()
{
    return reinterpret_cast<uintptr_t>(paging_table);
}

void PagingTableKernel::append_page_table()
{
}
