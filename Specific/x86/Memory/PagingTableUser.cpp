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

#include "PagingTableUser.h"

#include <art_string.h>
#include <memory.h>

bool PagingTableUser::v_addr_is_used(const virtual_address_t v_addr)
{
    const size_t dir_idx = v_addr.page_directory_index;
    if (!paging_directory[dir_idx].present) return false;
    auto [table] = *reinterpret_cast<page_table*>(paging_directory[dir_idx].page_table_entry_address << base_address_shift);

    return table->present;
}

virtual_address_t PagingTableUser::get_next_virtual_addr(const uintptr_t start_addr)
{
    auto working_addr = virtual_address_t(start_addr);
    while (v_addr_is_used(working_addr) && working_addr.raw < main_region_end)
    {
        working_addr.raw += page_alignment;
    }
    return working_addr;
}

virtual_address_t PagingTableUser::get_next_virtual_chunk(const uintptr_t start_addr, const size_t n_pages)
{
    virtual_address_t ret_addr = get_next_virtual_addr(start_addr);
    auto working_addr = ret_addr;
    size_t n_sequential = 0;
    while (n_sequential < n_pages && working_addr.raw < main_region_end)
    {
        working_addr.raw += page_alignment;
        if (v_addr_is_used(working_addr))
        {
            ret_addr = get_next_virtual_addr(working_addr.raw);
            working_addr = ret_addr;
            continue;
        }
        n_sequential++;
    }
    if (n_sequential == n_pages)return ret_addr;
    return virtual_address_t{0};
}

uintptr_t PagingTableUser::get_phys_from_virtual(uintptr_t v_addr)
{
    const virtual_address_t lookup = {v_addr};

    auto [table] = *reinterpret_cast<page_table*>(paging_directory[lookup.page_directory_index].page_table_entry_address << base_address_shift);

    if (const auto entry = table[lookup.page_table_index]; entry.present)
    {
        return entry.physical_address;
    }
    return 0;
}

page_table_entry_t PagingTableUser::check_vmap_contents(uintptr_t v_addr)
{
    const virtual_address_t lookup = {v_addr};
    auto [table] = *reinterpret_cast<page_table*>(paging_directory[lookup.page_directory_index].page_table_entry_address << base_address_shift);

    if (const auto entry = table[lookup.page_table_index]; entry.present)
    {
        return entry;
    }
    return page_table_entry_t{};
}

bool PagingTableUser::dir_entry_present(const size_t idx)
{
    return paging_directory[idx].present;
}

uintptr_t PagingTableUser::get_page_table_addr()
{
    return reinterpret_cast<uintptr_t>(paging_directory);
}

void* PagingTableUser::mmap(const uintptr_t addr, const size_t length, int prot, int flags, int fd, size_t offset)
{
    const size_t num_pages = (length + page_alignment - 1) >> base_address_shift;
    bool writeable = true; // TODO: get from flags
    const virtual_address_t ret_addr = get_next_virtual_chunk(addr, num_pages);
    if (ret_addr.raw == 0) return nullptr;
    virtual_address_t working_addr = ret_addr;
    for (size_t i = 0; i < num_pages; i++)
    {
        if (!dir_entry_present(working_addr.page_directory_index))
        {
            // TODO: handle flag ints as bools here
            append_page_table(writeable);
        }

        if (const auto phys_addr = page_get_next_phys_addr(); phys_addr != 0)
        {
            assign_page_table_entry(
                phys_addr,
                working_addr,
                writeable,
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

int PagingTableUser::munmap(void* addr, const size_t length_bytes)
{
    return unassign_page_table_entries(
        reinterpret_cast<uintptr_t>(addr) >> base_address_shift,
        (length_bytes + page_alignment - 1) >> base_address_shift); // this rounds up
}

page_table* PagingTableUser::append_page_table(const bool writable)
{
    const auto new_table_loc = reinterpret_cast<uintptr_t>(art_alloc(sizeof(page_table), page_alignment));
    size_t dir_idx = 0;
    for (; dir_idx < page_table_len && !paging_directory[dir_idx].present; dir_idx++)
    {
        // empty
    }
    paging_directory[dir_idx] = {
        1,
        1,
        1,
        0,
        0,
        0,
        0,
        0,
        0,
        new_table_loc >> base_address_shift,
    };
    return reinterpret_cast<page_table*>(new_table_loc);
}

void PagingTableUser::assign_page_table_entry(const uintptr_t physical_addr, const virtual_address_t v_addr, const bool writable, const bool)
{
    page_table_entry_t tab_entry = {};
    tab_entry.present = true;
    tab_entry.physical_address = physical_addr >> base_address_shift;
    tab_entry.rw = writable;
    tab_entry.user_access = true;
    auto [table] = *reinterpret_cast<page_table*>(paging_directory[v_addr.page_directory_index].page_table_entry_address << base_address_shift);
    table[v_addr.page_table_index] = tab_entry;
}

int PagingTableUser::unassign_page_table_entries(size_t start_idx, size_t n_pages)
{
    for (size_t i = start_idx; i < start_idx + n_pages; i++)
    {
        auto [table] = *reinterpret_cast<page_table*>(paging_directory[i / 1024].page_table_entry_address << base_address_shift);
        auto* tab_entry = &table[i % 1024];
        if (!tab_entry->present)return -1;

        const size_t phys_idx = tab_entry->physical_address;

        set_physical_bitmap_idx(phys_idx, true);
        tab_entry->raw = 0;
    }
    return 0;
}
