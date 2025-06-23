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

extern page_directory_4kb_t boot_page_directory[];
extern page_table boot_page_tables[];

bool PagingTableUser::v_addr_is_used(const virtual_address_t v_addr)
{
    const size_t dir_idx = v_addr.page_directory_index;
    if (!paging_directory[dir_idx].present) return false;
    auto [table] = *reinterpret_cast<page_table*>(paging_directory[dir_idx].page_table_entry_address << base_address_shift);

    return table->present;
}

void PagingTableUser::map_all_kernel_pages()
{
    // Copies all the mappings from kernel space into this directory. The contained addresses are all physical memory.
    art_string::memcpy(&paging_directory[768], &boot_page_directory[768], 256 * sizeof(page_directory_4kb_t));
    // art_string::memcpy(&paging_table[768], &boot_page_tables[768], 256 * sizeof(page_table));
}

PagingTableUser::PagingTableUser()
{
    paging_directory = static_cast<page_directory_4kb_t*>(art_alloc(sizeof(page_directory_4kb_t) * 1024, page_alignment));
    art_string::memset(paging_directory, 0, 1024 * sizeof(page_directory_4kb_t));
    // Shares kernel tables at the top end, so only need bottom 768 for each user space program
    paging_table = static_cast<page_table*>(art_alloc(sizeof(page_table) * 768, page_alignment));
    art_string::memset(paging_table, 0, 768 * sizeof(page_table));

    // TODO: for each page directory, set the table location
    map_all_kernel_pages();
    for (size_t i = 0; i < 768; i++)
    {
        paging_directory[i].page_table_entry_address = PagingTableUser::get_phys_from_virtual(reinterpret_cast<uintptr_t>(&paging_table[i]));
    }

    // uintptr_t addr = PagingTableUser::get_phys_addr_of_page_dir();
    // asm volatile("mov %0, %%cr3" :: "r"(addr) : "memory");
}

PagingTableUser::~PagingTableUser()
{
    art_free(paging_directory);
    art_free(paging_table);
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


uintptr_t PagingTableUser::get_phys_addr_of_page_dir()
{
    return kget_mapping_target(paging_directory);
}


void* PagingTableUser::mmap(const uintptr_t addr, const size_t length, int prot, int flags, int fd, size_t offset)
{
    const size_t num_pages = (length + page_alignment - 1) >> base_address_shift;
    bool writeable = flags & PAGING_WRITABLE; // TODO: get from flags
    const virtual_address_t ret_addr = get_next_virtual_chunk(addr, num_pages);
    if (ret_addr.raw == 0) return nullptr;
    virtual_address_t working_addr = ret_addr;
    for (size_t i = 0; i < num_pages; i++)
    {
        if (!dir_entry_present(working_addr.page_directory_index))
        {
            // TODO: handle flag ints as bools here
            append_page_table(writeable, true);
        }

        if (uintptr_t phys_addr = page_get_next_phys_addr(); phys_addr != 0)
        {
            assign_page_table_entries(
                phys_addr,
                working_addr.raw,
                writeable, true
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

page_table* PagingTableUser::append_page_table(const bool writable, const bool user)
{
    // TODO: get next free instead

    return nullptr;
    // const auto new_table_loc = reinterpret_cast<uintptr_t>(art_alloc(sizeof(page_table), page_alignment));
    // size_t dir_idx = 0;
    // for (; dir_idx < page_table_len && !paging_directory[dir_idx].present; dir_idx++)
    // {
    //     // empty
    // }
    // paging_directory[dir_idx] = {
    //     1,
    //     writable,
    //     user,
    //     0,
    //     0,
    //     0,
    //     0,
    //     0,
    //     0,
    //     new_table_loc >> base_address_shift, // TODO: this is meant to be a physical memory address
    // };
    // return reinterpret_cast<page_table*>(new_table_loc);
}


void PagingTableUser::assign_page_table_entries(const uintptr_t physical_addr, const uintptr_t virt_addr, const bool writable, const bool user)
{
    auto v_addr = virtual_address_t{virt_addr};
    auto tab_entry = page_table_entry_t{0};
    tab_entry.present = true;
    tab_entry.physical_address = physical_addr >> base_address_shift;
    tab_entry.rw = writable;
    tab_entry.user_access = true;
    paging_directory[v_addr.page_directory_index].present = true;
    paging_directory[v_addr.page_directory_index].rw |= writable;
    paging_directory[v_addr.page_directory_index].user_access = true;
    paging_table[v_addr.page_directory_index].table[v_addr.page_table_index] = tab_entry;
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

int PagingTableUser::map_kernel_page(uintptr_t physicaL_page_addr, uintptr_t virtual_page_addr)
{
}

bool PagingTableUser::dir_entry_present(const size_t idx)
{
    return paging_directory[idx].present;
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
