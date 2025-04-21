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
#include "errno.h"

#include "multiboot2.h"
#include "logging.h"
#include "string.h"
#include "cmp_int.h"
#include "DenseBooleanArray.h"
#include "Scheduler.h"


constexpr size_t max_n_pages = 0x100000;

/// Each table is 4k in size, and is page aligned i.e. 4k aligned. They consists of 1024 32 bit entries.
constexpr size_t page_table_size = 1024;


struct page_table
{
    page_table_entry_t table[page_table_size];
};

uintptr_t main_region_start;
uintptr_t main_region_end;
size_t last_physical_idx;
page_directory_4kb_t paging_directory[page_table_size]__attribute__((aligned(page_alignment)));
page_table kernel_paging_tables[page_table_size]__attribute__((aligned(page_alignment)));
uintptr_t user_base_address = 256 * 1024 * 1024; // 256MB of kernel space?

// bitmaps used to keep track of the next virtual and physical pages available.
// These are the same during identity mapping, but diverge when user space programs make malloc calls
// 4GB worth of 4096 pages. Set all to false. Processing the multiboot2 memory_map will set the necessary bits.
constexpr size_t n_DBs = (max_n_pages + (64 - 1)) / 32;
u64 paging_phys_bitmap_array[n_DBs];
u64 paging_virt_bitmap_array[n_DBs];

DenseBooleanArray<u64> page_available_physical_bitmap;
DenseBooleanArray<u64> page_available_virtual_bitmap;

// TODO: replace target_pid workaround with proper file_descriptor handling in malloc.
// size_t target_pid = -1; // for use with kmalloc only. Allows scheduler to allocate userspace memory.


// TODO: helper functions like converters between addresses and page numbers?

uintptr_t page_get_next_phys_addr()
{
    const size_t idx = page_available_physical_bitmap.get_next_true();
    if (idx == DBA_ERR_IDX) return 0;
    return idx << base_address_shift;
}

// TODO: Should go back to 0 if doesn't find anything above start_addr
uintptr_t page_get_next_virtual_chunk(size_t idx, const size_t n_pages)
{
    idx = page_available_virtual_bitmap.get_next_trues(idx, n_pages);
    if (idx == DBA_ERR_IDX) return 0;
    return idx << base_address_shift;
}

// TODO: Should go back to 0 if doesn't find anything above start_addr
uintptr_t page_get_next_virt_addr(const uintptr_t start_addr)
{
    const size_t idx = page_available_virtual_bitmap.get_next_true(start_addr >> base_address_shift);
    if (idx == DBA_ERR_IDX) return 0;
    return idx << base_address_shift;
}


void assign_page_directory_entry(size_t dir_idx, const bool writable, const bool user)
{
    page_directory_4kb_t dir_entry;
    dir_entry.page_table_entry_address = reinterpret_cast<uintptr_t>(&kernel_paging_tables[dir_idx]) >> base_address_shift; // might be wrong.
    dir_entry.rw = writable;
    dir_entry.user_access = user;
    paging_directory[dir_idx] = dir_entry;
}

void assign_page_table_entry(const uintptr_t physical_addr, const virtual_address_t v_addr, const bool writable, const bool user)
{
    page_table_entry_t tab_entry = {};
    tab_entry.present = true;
    tab_entry.physical_address = physical_addr >> base_address_shift;
    tab_entry.rw = writable;
    tab_entry.user_access = user;

    kernel_paging_tables[v_addr.page_directory_index].table[v_addr.page_table_index] = tab_entry;

    page_available_physical_bitmap.set_bit(physical_addr >> base_address_shift, false);
    page_available_virtual_bitmap.set_bit(v_addr.raw >> base_address_shift, false);
}

// TODO: do we need to unassign directories? Probably not.

int unassign_page_table_entries(const size_t start_idx, const size_t n_pages)
{
    for (size_t i = start_idx; i < start_idx + n_pages; i++)
    {
        auto* tab_entry = &kernel_paging_tables[i / 1024].table[i % 1024];
        if (!tab_entry->present)return -1;

        const size_t phys_idx = tab_entry->physical_address;

        page_available_physical_bitmap.set_bit(phys_idx, true);
        tab_entry->raw = 0;
    }
    page_available_virtual_bitmap.set_range(start_idx, n_pages, true);

    return 0;
}


void paging_identity_map(uintptr_t phys_addr, const size_t size, const bool writable, const bool user)
{
    virtual_address_t virtual_address = {.raw = phys_addr};

    const size_t num_pages = (size + page_alignment - 1) >> base_address_shift;
    for (size_t i = 0; i < num_pages; i++)
    {
        // Every 1024 table entries requires a new dir entry.

        if (!paging_directory[virtual_address.page_directory_index].present)
        {
            assign_page_directory_entry(virtual_address.page_directory_index, writable, user);
        }
        if (!kernel_paging_tables[virtual_address.page_directory_index].table[virtual_address.page_table_index].present)
        {
            assign_page_table_entry(phys_addr, virtual_address, writable, user);
        }

        phys_addr += page_alignment;
        virtual_address.raw = phys_addr;
        if (virtual_address.raw >> 12 >= max_n_pages) return;
    }
}

void enable_paging()
{
    auto addr = reinterpret_cast<u32>(&paging_directory[0]) | 0xFFF;
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
    // TODO:  This should call PagingTableKernel() which should do this
    // Iniitalise bitmaps
    page_available_virtual_bitmap.init(paging_virt_bitmap_array, max_n_pages, true);
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
            paging_identity_map(last_end, entry->addr - last_end, true, false);
        }

        if (entry->addr < brk_loc && entry->addr + entry->len > brk_loc)
        {
            // contains kernel
            // only map used kernel region.
            paging_identity_map(entry->addr, brk_loc - entry->addr, entry->type == 1 && entry->addr > 0, false);
            main_region_start = entry->addr;
            main_region_end = entry->addr + entry->len;
            last_physical_idx = main_region_end >> base_address_shift;
        }
        else
        {
            paging_identity_map(entry->addr, entry->len, entry->type == 1 && entry->addr > 0, false);
        }

        last_end = entry->addr + entry->len;
    }

    // Protect kernel and init identity map
    // paging_identity_map(main_region_start, post_kernel_page - main_region_start, true, false);
    paging_identity_map(0xf0000000, 0xffffffff - 0xf0000000, true, false);

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
    // mmap should return a contiguous chunk of virtual memory, so some checks should be done first.
    // if (addr < main_region_start) addr = main_region_start;
    // This should call PagingTable->mmap

    // bool is_user = false;
    // if (target_pid != -1)
    // {
    //     is_user = Scheduler::isProcessUser(target_pid);
    //     target_pid = -1;
    // }
    // else
    // {
    //     is_user = Scheduler::isCurrentProcessUser();
    // }

    // if (is_user)
    // {
    //     addr = MAX(addr, user_base_address); // start trying to allocate at user base address for user applications.
    // }

    const size_t first_page = addr >> base_address_shift;
    const size_t num_pages = (length + page_alignment - 1) >> base_address_shift;

    const virtual_address_t ret_addr = {page_get_next_virtual_chunk(first_page, num_pages)};
    virtual_address_t working_addr = ret_addr;
    for (size_t i = 0; i < num_pages; i++)
    {
        if (!paging_directory[working_addr.page_directory_index].present)
        {
            // TODO: handle flag ints as bools here
            assign_page_directory_entry(working_addr.page_directory_index, true, false);
        }


        if (const auto phys_addr = page_get_next_phys_addr(); phys_addr != 0)
        {
            assign_page_table_entry(
                phys_addr,
                working_addr,
                true,
                false
            );
        }
        else
        {
            return nullptr;
        }

        working_addr.raw += page_alignment;
    }

    const auto p = reinterpret_cast<void*>(ret_addr.raw);
    memset(p, 0, length);
    return p;
}

int kmunmap(void* addr, const size_t length_bytes)
{
    // TODO: More checks such as "you don't own this memory"
    return unassign_page_table_entries(
        reinterpret_cast<uintptr_t>(addr) >> base_address_shift,
        (length_bytes + page_alignment - 1) >> base_address_shift); // this rounds up
}

uintptr_t paging_get_phys_addr(uintptr_t vaddr)
{
    const virtual_address_t lookup = {vaddr};
    if (const auto entry = kernel_paging_tables[lookup.page_directory_index].table[lookup.page_table_index]; entry.present)
    {
        return entry.physical_address;
    }
    return 0;
}

page_table_entry_t paging_check_contents(uintptr_t vaddr)
{
    const virtual_address_t lookup = {vaddr};
    if (const auto entry = kernel_paging_tables[lookup.page_directory_index].table[lookup.page_table_index]; entry.present)
    {
        return entry;
    }
    return page_table_entry_t{};
}

//
// void paging_set_target_pid(size_t pid)
// {
//     target_pid = pid;
// }


// uintptr_t paging_check_contents(void* vaddr)
// {
//     return paging_check_contents(reinterpret_cast<uintptr_t>(vaddr));
// }
