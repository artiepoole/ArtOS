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
// Created by artiepoole on 4/29/25.
//

#include <PagingTable.h>

uintptr_t PagingTable::get_page_table_addr()
{
    return reinterpret_cast<uintptr_t>(paging_directory);
}

uintptr_t PagingTable::page_get_next_virtual_chunk(size_t idx, size_t n_pages)
{
    idx = page_available_virtual_bitmap->get_next_trues(idx, n_pages);
    if (idx == DBA_ERR_IDX) return 0;
    return idx << base_address_shift;
}

uintptr_t PagingTable::page_get_next_virtual_addr(uintptr_t start_addr)
{
    const size_t idx = page_available_virtual_bitmap->get_next_true(start_addr >> base_address_shift);
    if (idx == DBA_ERR_IDX) return 0;
    return idx << base_address_shift;
}

bool PagingTable::dir_entry_present(const size_t idx)
{
    return paging_directory[idx].present;
}


uintptr_t PagingTable::get_phys_from_virtual(uintptr_t v_addr)
{
    const virtual_address_t lookup = {v_addr};
    if (const auto entry = paging_tables[lookup.page_directory_index].table[lookup.page_table_index]; entry.present)
    {
        return entry.physical_address;
    }
    return 0;
}

page_table_entry_t PagingTable::check_vmap_contents(uintptr_t v_addr)
{
    const virtual_address_t lookup = {v_addr};
    if (const auto entry = paging_tables[lookup.page_directory_index].table[lookup.page_table_index]; entry.present)
    {
        return entry;
    }
    return page_table_entry_t{};
}


void PagingTable::assign_page_table_entry(const uintptr_t physical_addr, const virtual_address_t v_addr, const bool writable, const bool user)
{
    // TODO: This should definitely be a method of the class.
    page_table_entry_t tab_entry = {};
    tab_entry.present = true;
    tab_entry.physical_address = physical_addr >> base_address_shift;
    tab_entry.rw = writable;
    tab_entry.user_access = user;
    paging_tables[v_addr.page_directory_index].table[v_addr.page_table_index] = tab_entry;
    page_available_virtual_bitmap->set_bit(v_addr.raw >> base_address_shift, false);
    set_physical_bitmap_addr(physical_addr, false);
}

int PagingTable::unassign_page_table_entries(const size_t start_idx, const size_t n_pages)
{
    // TODO: mark empty dicts as not present?
    for (size_t i = start_idx; i < start_idx + n_pages; i++)
    {
        auto* tab_entry = &paging_tables[i / 1024].table[i % 1024];
        if (!tab_entry->present)return -1;

        const size_t phys_idx = tab_entry->physical_address;

        set_physical_bitmap_idx(phys_idx, true);
        tab_entry->raw = 0;
    }
    page_available_virtual_bitmap->set_range(start_idx, n_pages, true);

    return 0;
}
