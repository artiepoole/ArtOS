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
// Created by artypoole on 13/07/24.
//

#ifndef MEMORY_H
#define MEMORY_H


#include "types.h"

// http://wiki.osdev.org/Memory_Map_(x86)
// "Use the BIOS function INT 15h, EAX=0xE820 to get a reliable map of Extended Memory."
// http://wiki.osdev.org/Detecting_Memory_(x86)#BIOS_Function:_INT_0x15,_EAX_=_0xE820
#ifdef __cplusplus
extern "C" {
void art_memory_init();

// Allocate call used by kernel processes
void* art_alloc(size_t size_bytes, size_t alignment_size = 0, int flags = 0);

// Free call used by kernel processes.
void art_free(const void* ptr);


#endif

enum ART_ALLOC_FLAGS
{
    KERNEL_SPACE,
    USER_SPACE
};

#ifdef __cplusplus
}
#endif
#endif //MEMORY_H
