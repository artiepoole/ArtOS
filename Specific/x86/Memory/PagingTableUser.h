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
    PagingTableUser();
    void* mmap(uintptr_t addr, size_t length, int prot, int flags, int fd, size_t offset) override { return nullptr; }
    int munmap(void* addr, size_t length_bytes) override { return 0; }
    void append_page_table(bool writable);

    // TODO: WAIT I PROBABLY DON'T WANT A DBA FOR USER SPACE APPS!!!!!!!!!!!!!!!
    // TODO: This means I need to move the generic logic in PagingTable.h/.cpp to implementation specific stuff.

private:
    u64 paging_virt_bitmap_array[paging_bitmap_n_DBs];
    DenseBooleanArray<u64> page_available_virtual_bitmap_instance;
};


#endif //PAGINGTABLEUSER_H
