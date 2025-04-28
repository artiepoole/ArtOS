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

#ifndef PAGINGTABLE_H
#define PAGINGTABLE_H
#include <stdint.h>
#include "paging.h"
#include "DenseBooleanArray.h"


/**
 *
 */
class PagingTable
{
    /* This needs to handle the processes memmory map.
     * It should be used by the scheduler to update the paging tables on malloc and free
     * It should have an init call which calls art_alloc to get somewhere to store its paging tables
     * It should have a destructor which frees those tables
     * The paging system in paging.cpp should use the "current process" to get the start of the paging
     *  tables and it should run through them to find the appropriate entries.
     *
    */
public:
    virtual ~PagingTable() = default;
    virtual void* mmap(uintptr_t addr, size_t length, int prot, int flags, int fd, size_t offset) = 0;
    virtual int munmap(void* addr, size_t length_bytes) = 0;
    uintptr_t get_page_table_addr();
    uintptr_t page_get_next_virtual_chunk(size_t idx, size_t n_pages);
    uintptr_t page_get_next_virtual_addr(uintptr_t start_addr);
    bool dir_entry_present(size_t idx);
    uintptr_t get_phys_from_virtual(uintptr_t v_addr);
    page_table_entry_t check_vmap_contents(uintptr_t v_addr);
    void assign_page_table_entry(uintptr_t physical_addr, virtual_address_t v_addr, bool writable, bool user);
    int unassign_page_table_entries(size_t start_idx, size_t n_pages);

protected:
    DenseBooleanArray<u64>* page_available_virtual_bitmap;
    page_directory_4kb_t* paging_directory = nullptr;
    page_table* paging_tables = nullptr;
};


#endif //PAGINGTABLE_H
