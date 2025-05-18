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

#ifndef CPPMEMORY_H
#define CPPMEMORY_H

#include "types.h"

namespace std {
    enum class align_val_t : size_t {}; // define std::align_val_t if missing
}


void* operator new(size_t size);

void* operator new[](size_t size);
void* operator new(size_t size, std::align_val_t);

void operator delete(void* p);
void operator delete(void* p, size_t);
void operator delete(void* p, size_t, std::align_val_t);

void operator delete[](void* p);
void operator delete[](void* p, size_t);


#endif //CPPMEMORY_H
