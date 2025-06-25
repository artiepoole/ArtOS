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
extern page_directory_4kb_t boot_page_directory[];
extern page_table boot_page_tables[];
// PagingTableKernel kernel_pages;

PagingTableKernel& kernel_pages()
{
    static PagingTableKernel instance;
    return instance;
}

uintptr_t main_region_start;
uintptr_t main_region_end;
size_t last_physical_idx;

extern unsigned char kernel_start;
unsigned char* kernel_start_loc = &kernel_start;


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

uintptr_t get_kernal_page_dir()
{
    return kernel_pages().get_phys_addr_of_page_dir();
}

void enable_paging()
{
    auto addr = kernel_pages().get_phys_addr_of_page_dir() | 0xFFF; // TODO: shouldn't this be & 0xfffff000 ?
    __asm__ volatile ("mov %0, %%cr3" : : "r"(addr)); // set the cr3 to the paging_directory physical address
    u32 cr0 = 0;
    __asm__ volatile ("mov %%cr0, %0" : "=r"(cr0));
    cr0 = cr0 | 0x80000001;
    __asm__ volatile ("mov %0, %%cr0" : : "r"(cr0));
}

// TODO: need to map entry addresses
/** Quickly and unsafely identity map a memory region
 *
 * @param start
 * @param end
 */
void dirty_ident_map(const uintptr_t start, const uintptr_t end)
{
    const size_t n_pages = (end - start + page_alignment - 1) >> base_address_shift;
    virtual_address_t vaddr = {start};
    for (size_t i = 0; i < n_pages; i++)
    {
        boot_page_tables[vaddr.page_directory_index].table[vaddr.page_table_index].raw = (vaddr.raw & 0xfffff000) | 0x3;
        boot_page_directory[vaddr.page_directory_index].raw = (reinterpret_cast<uintptr_t>(&boot_page_tables[vaddr.page_directory_index]) - 0xc0000000) | 0x3;
        vaddr.raw += 4096;
    }
}


/**
 * Qiuckly and unsafely unmap an identity mapped region
 * @param start
 * @param end
 */
void dirty_ident_unmap(const uintptr_t start, const uintptr_t end)
{
    const size_t n_pages = (end - start + page_alignment - 1) >> base_address_shift;
    virtual_address_t vaddr = {start};
    for (size_t i = 0; i < n_pages; i++)
    {
        boot_page_tables[vaddr.page_directory_index].table[vaddr.page_table_index].raw = 0;
        boot_page_directory[vaddr.page_directory_index].raw = 0;
        vaddr.raw += 4096;
    }
}


/*
 *  Uses kernel_brk because this is the last memory address to be given to the kernel.
 *  This could use kernel end once sbrk is no longer a thing.
 */
void mmap_init(multiboot2_tag_mmap* mmap)
{
    page_available_physical_bitmap.init(paging_phys_bitmap_array, max_n_pages, false);

    const auto brk_loc = reinterpret_cast<uintptr_t>(kernel_brk);
    const size_t n_entries = mmap->size / sizeof(multiboot2_mmap_entry);
    const uintptr_t post_kernel_page = ((brk_loc >> base_address_shift) + 1) << base_address_shift; // first page after kernel image.

    // TODO: I Don't want it identitiy map if type 1!
    // Loop through all entries and map appropriately
    dirty_ident_map(reinterpret_cast<uintptr_t>(mmap->entries[0]), reinterpret_cast<uintptr_t>(mmap->entries[5]) + sizeof(multiboot2_mmap_entry));
    for (size_t i = 0; i < n_entries; i++)
    {
        multiboot2_mmap_entry const* entry = mmap->entries[i];
        if (entry->type == 1) continue;

        if (entry->addr < brk_loc && entry->addr + entry->len > brk_loc)
        {
            // contains kernel
            // only map used kernel region.
            kernel_pages().identity_map(entry->addr, brk_loc - entry->addr, false, false);
            main_region_start = entry->addr;
            main_region_end = entry->addr + entry->len;
            last_physical_idx = main_region_end >> base_address_shift;
        }
        else
        {
            kernel_pages().identity_map(entry->addr, entry->len, false && entry->addr > 0, false);
        }

    }
    dirty_ident_unmap(reinterpret_cast<uintptr_t>(mmap->entries[0]), reinterpret_cast<uintptr_t>(mmap->entries[5]) + sizeof(multiboot2_mmap_entry));

    // Protect kernel and init identity map
    // paging_identity_map(main_region_start, post_kernel_page - main_region_start, true, false);
    // kernel_pages().identity_map(0xf0000000, 0xffffffff - 0xf0000000, true, false);

    // set upper limit in physical bitmap to extents of the avaialble
    // TODO: this is not correct at all.
    // page_available_physical_bitmap.set_range(0, post_kernel_page-0xc0000000, false);
    page_available_physical_bitmap.set_range(
        (post_kernel_page - 0xc0000000) >> base_address_shift,
        (main_region_end - post_kernel_page - 0xc0000000) >> base_address_shift,
        true
    );

    kernel_pages().reserve_kernel_v_addr_space(reinterpret_cast<void*>(0xc0000000), kernel_brk);


    LOG("Paging: memory map processed.");
    enable_paging();
    LOG("Paging: paging enabled.");
}


void* kmmap(uintptr_t addr, size_t length, int prot, int flags, int fd, size_t offset)
{
    return kernel_pages().mmap(addr, length, prot, flags, fd, offset);
}

uintptr_t kget_mapping_target(void* v_addr)
{
    return kernel_pages().get_phys_from_virtual(reinterpret_cast<uintptr_t>(v_addr)) * page_alignment;
}

int kmunmap(void* addr, const size_t length_bytes)
{
    return kernel_pages().munmap(addr, length_bytes);
}

void paging_identity_map(const uintptr_t phys_addr, const size_t size, const bool writable, const bool user)
{
    kernel_pages().identity_map(phys_addr, size, writable, user);
}
