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
// Created by artypoole on 09/09/24.
//

#include "CPPMemory.h"
#include "memory.h"


void* operator new(size_t size) throw()
{
    if (size == 0) size = 1;

    return art_alloc(size);
}


void* operator new[](size_t size) throw()
{
    if (size == 0) size = 1;

    return art_alloc(size, 0);
}


void operator delete(void* p) throw()
{
    if (p) art_free(p);
}


void operator delete(void* ptr, size_t ) noexcept {
    // optional — if alignment matters, handle here
    operator delete(ptr);
}

void operator delete(void* p, size_t, std::align_val_t) throw()
{
    if (p) art_free(p);
}

void operator delete[](void* p) throw()
{
    if (p) art_free(p);
}

void operator delete[](void* p, size_t) throw()
{
    if (p) art_free(p);
}
