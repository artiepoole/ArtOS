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
// Created by artypoole on 18/11/24.
//

#include "memory.h"
#include "paging.h"

#include <PagingTableKernel.h>

#include "errno.h"

#include "multiboot2.h"
#include "logging.h"
#include "string.h"
#include "cmp_int.h"
#include "DenseBooleanArray.h"
#include "Scheduler.h"


/// Each table is 4k in size, and is page aligned i.e. 4k aligned. They consists of 1024 32 bit entries.
constexpr size_t page_table_size = 1024;

// PagingTableKernel kernel_pages;

uintptr_t main_region_start;
uintptr_t main_region_end;
size_t last_physical_idx;


// bitmaps used to keep track of the next virtual and physical pages available.
// These are the same during identity mapping, but diverge when user space programs make malloc calls
// 4GB worth of 4096 pages. Set all to false. Processing the multiboot2 memory_map will set the necessary bits.

u64 paging_phys_bitmap_array[paging_bitmap_n_DBs];
DenseBooleanArray<u64> page_available_physical_bitmap;


uintptr_t page_get_next_phys_addr()
{
    const size_t idx = page_available_physical_bitmap.get_next_true();
    if (idx == DBA_ERR_IDX) return 0;
    return idx << base_address_shift;
}


void set_physical_bitmap_addr(const uintptr_t physical_addr, const bool state)
{
    page_available_physical_bitmap.set_bit(physical_addr >> base_address_shift, state);
}

void set_physical_bitmap_idx(const size_t phys_idx, const bool state)
{
    page_available_physical_bitmap.set_bit(phys_idx, state);
}

void enable_paging()
{
    auto addr = get_kernel_pages().get_page_table_addr() | 0xFFF;
    __asm__ volatile ("mov %0, %%cr3" : : "r"(addr)); // set the cr3 to the paging_directory physical address
    u32 cr0 = 0;
    __asm__ volatile ("mov %%cr0, %0" : "=r"(cr0));
    cr0 = cr0 | 0x80000001;
    __asm__ volatile ("mov %0, %%cr0" : : "r"(cr0));
}

/*
 *  Uses kernel_brk because this is the last memory address to be given to the kernel.
 *  This could use kernel end once sbrk is no longer a thing.
 */
void mmap_init(multiboot2_tag_mmap* mmap)
{
    // get_kernel_pages().late_init();
    // kernel_pages.late_init();
    // TODO:  This should call PagingTableKernel() which should do this
    // Iniitalise bitmaps

    page_available_physical_bitmap.init(paging_phys_bitmap_array, max_n_pages, false);


    const auto brk_loc = reinterpret_cast<uintptr_t>(kernel_brk);
    const size_t n_entries = mmap->size / sizeof(multiboot2_mmap_entry);
    const uintptr_t post_kernel_page = ((brk_loc >> base_address_shift) + 1) << base_address_shift; // first page after kernel image.
    uintptr_t last_end = 0;

    // Loop through all entries and map appropriately
    for (size_t i = 0; i < n_entries; i++)
    {
        multiboot2_mmap_entry const* entry = mmap->entries[i];
        if (entry->addr > last_end)
        {
            // fill holes
            get_kernel_pages().paging_identity_map(last_end, entry->addr - last_end, true, false);
        }

        if (entry->addr < brk_loc && entry->addr + entry->len > brk_loc)
        {
            // contains kernel
            // only map used kernel region.
            get_kernel_pages().paging_identity_map(entry->addr, brk_loc - entry->addr, entry->type == 1 && entry->addr > 0, false);
            main_region_start = entry->addr;
            main_region_end = entry->addr + entry->len;
            last_physical_idx = main_region_end >> base_address_shift;
        }
        else
        {
            get_kernel_pages().paging_identity_map(entry->addr, entry->len, entry->type == 1 && entry->addr > 0, false);
        }

        last_end = entry->addr + entry->len;
    }

    // Protect kernel and init identity map
    // paging_identity_map(main_region_start, post_kernel_page - main_region_start, true, false);
    get_kernel_pages().paging_identity_map(0xf0000000, 0xffffffff - 0xf0000000, true, false);

    // set upper limit in physical bitmap to extents of the avaialble
    page_available_physical_bitmap.set_range(
        post_kernel_page >> base_address_shift,
        (main_region_end - post_kernel_page) >> base_address_shift,
        true
    );


    LOG("Paging: memory map processed.");
    enable_paging();
    LOG("Paging: paging enabled.");
}


void* kmmap(uintptr_t addr, size_t length, int prot, int flags, int fd, size_t offset)
{
    return get_kernel_pages().mmap(addr, length, prot, flags, fd, offset);
}

int kmunmap(void* addr, const size_t length_bytes)
{
    return get_kernel_pages().munmap(addr, length_bytes);
}

void paging_identity_map(const uintptr_t phys_addr, const size_t size, const bool writable, const bool user)
{
    get_kernel_pages().paging_identity_map(phys_addr, size, writable, user);
}
