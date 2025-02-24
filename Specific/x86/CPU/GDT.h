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
// Created by artypoole on 25/11/24.
//

#ifndef GDT_H
#define GDT_H

constexpr u16 null_offset = 0x0;
constexpr u16 kernel_cs_offset = 0x8;
constexpr u16 kernel_ds_offset = 0x10;
constexpr u16 user_cs_offset = 0x18;
constexpr u16 user_ds_offset = 0x20;
constexpr u16 tss_offset = 0x28;

void GDT_init();


#endif //GDT_H
