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

#include <art_string.h>
#include <PagingTable.h>


// constexpr size_t max_n_pages = 0x100000;


uintptr_t user_base_address = 256 * 1024 * 1024; // 256MB of kernel space?

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


void PagingTableKernel::late_init()
{
    page_available_virtual_bitmap_instance.init(paging_virt_bitmap_array, max_n_pages, true);
    page_available_virtual_bitmap = &page_available_virtual_bitmap_instance;
    paging_directory = paging_directory_data;
    paging_tables = paging_table_data;
}


void PagingTableKernel::assign_page_directory_entry(const size_t dir_idx, const bool writable, const bool user)
{
    page_directory_4kb_t dir_entry;
    dir_entry.page_table_entry_address = reinterpret_cast<uintptr_t>(&paging_tables[dir_idx]) >> base_address_shift; // might be wrong.
    dir_entry.rw = writable;
    dir_entry.user_access = user;
    paging_directory[dir_idx] = dir_entry;
}


void* PagingTableKernel::mmap(const uintptr_t addr, const size_t length, int prot, int flags, int fd, size_t offset)
{
    const size_t first_page = addr >> base_address_shift;
    const size_t num_pages = (length + page_alignment - 1) >> base_address_shift;

    const virtual_address_t ret_addr = {page_get_next_virtual_chunk(first_page, num_pages)};
    virtual_address_t working_addr = ret_addr;
    for (size_t i = 0; i < num_pages; i++)
    {
        if (!dir_entry_present(working_addr.page_directory_index))
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
    art_string::memset(p, 0, length);
    return p;
}


void PagingTableKernel::paging_identity_map(uintptr_t phys_addr, const size_t size, const bool writable, const bool user)
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
        if (!paging_tables[virtual_address.page_directory_index].table[virtual_address.page_table_index].present)
        {
            assign_page_table_entry(phys_addr, virtual_address, writable, user);
        }

        phys_addr += page_alignment;
        virtual_address.raw = phys_addr;
        if (virtual_address.raw >> 12 >= max_n_pages) return;
    }
}

// TODO: More checks such as "you don't own this memory"
int PagingTableKernel::munmap(void* addr, const size_t length_bytes)
{
    return unassign_page_table_entries(
        reinterpret_cast<uintptr_t>(addr) >> base_address_shift,
        (length_bytes + page_alignment - 1) >> base_address_shift); // this rounds up
}

