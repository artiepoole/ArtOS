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

#include <memory.h>

PagingTableUser::PagingTableUser()
{
    page_available_virtual_bitmap_instance.init(paging_virt_bitmap_array, max_n_pages, true);
    page_available_virtual_bitmap = &page_available_virtual_bitmap_instance;
    paging_directory = static_cast<page_directory_4kb_t*>(art_alloc(4096, page_alignment));
    paging_tables = static_cast<page_table*>(art_alloc(sizeof(page_table), page_alignment));
    paging_directory[0] = {
        1,
        1,
        1,
        0,
        0,
        0,
        0,
        0,
        0,
        reinterpret_cast<u32>(&paging_tables[0]) >> base_address_shift,
    };
}


void PagingTableUser::append_page_table(const bool writable)
{
}
