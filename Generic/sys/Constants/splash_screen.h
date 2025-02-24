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
// Created by artypoole on 27/06/24.
//

#ifndef SPLASH_SCREEN_H
#define SPLASH_SCREEN_H


#include "types.h"
inline u32 load_bar_region[4] = {248, 483, 534, 24}; // TODO: Calculate this instead.
inline u32 splash_logo_width = 540;
inline u32 splash_logo_height = 160;
extern u32 SPLASH_DATA[540 * 160];
#endif //SPLASH_SCREEN_H
