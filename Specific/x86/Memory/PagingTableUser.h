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

#ifndef PAGINGTABLEUSER_H
#define PAGINGTABLEUSER_H

#include "PagingTable.h"

class PagingTableUser : public PagingTable
{
public:
    PagingTableUser() = default;

    virtual_address_t get_next_virtual_addr(uintptr_t start_addr);
    virtual_address_t get_next_virtual_chunk(uintptr_t addr, size_t n_pages);
    uintptr_t get_phys_from_virtual(uintptr_t v_addr) override;
    page_table_entry_t check_vmap_contents(uintptr_t v_addr) override;
    bool dir_entry_present(size_t idx) override;
    uintptr_t get_page_table_addr() override;
    void* mmap(uintptr_t addr, size_t length, int prot, int flags, int fd, size_t offset) override;
    int munmap(void* addr, size_t length_bytes) override;

    page_table* append_page_table(bool writable);
    void assign_page_table_entry(u32 physical_addr, virtual_address_t v_addr, bool writable, bool user) override;
    int unassign_page_table_entries(size_t start_idx, size_t n_pages) override;

    // TODO: WAIT I PROBABLY DON'T WANT A DBA FOR USER SPACE APPS!!!!!!!!!!!!!!!
    // TODO: This means I need to move the generic logic in PagingTable.h/.cpp to implementation specific stuff.

private:
    page_directory_4kb_t paging_directory[page_table_len];
    bool v_addr_is_used(virtual_address_t v_addr);
};


#endif //PAGINGTABLEUSER_H
