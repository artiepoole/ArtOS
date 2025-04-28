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

class PagingTableUser : PagingTable
{
public:
    PagingTableUser();

    uintptr_t get_page_table_addr() override;
    void append_page_table(bool writable) override;

private:
    page_directory_4kb_t** paging_table = nullptr; // stores 1024 page tables
    page_table_entry_t** page_tables = nullptr; // stores 1024 mapping entries
    size_t n_tables = 0;
};


#endif //PAGINGTABLEUSER_H
