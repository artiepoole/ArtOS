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
// Created by artypoole on 04/10/24.
//

#ifndef SIMD_H
#define SIMD_H

#ifdef __cplusplus
extern "C" {
#endif
#include "_types.h"
void* simd_copy(void* dest, const void* src, size_t size);
void* simd_move(void* dest, const void* src, size_t n);
void* simd_set(void* dest, int value, size_t size);

#ifdef __cplusplus
}
#endif
#endif //SIMD_H
