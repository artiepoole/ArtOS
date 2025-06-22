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
    // virtual ~PagingTable() = default;
    PagingTable() = default;
    virtual ~PagingTable() = default;
    virtual void* mmap(uintptr_t addr, size_t length, int prot, int flags, int fd, size_t offset) = 0;
    virtual int munmap(void* addr, size_t length_bytes) = 0;
    virtual uintptr_t get_phys_addr_of_page_dir() = 0;
    virtual bool dir_entry_present(size_t idx) = 0;
    virtual uintptr_t get_phys_from_virtual(uintptr_t v_addr) = 0;
    virtual page_table_entry_t check_vmap_contents(uintptr_t v_addr) = 0;
    // virtual void assign_page_table_entry(uintptr_t physical_addr, virtual_address_t v_addr, bool writable, bool user) = 0;
    virtual int unassign_page_table_entries(size_t start_idx, size_t n_pages) = 0;
};


#endif //PAGINGTABLE_H
