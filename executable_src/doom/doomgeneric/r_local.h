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

#ifndef __R_LOCAL__
#define __R_LOCAL__

// Binary Angles, sine/cosine/atan lookups.
#include "tables.h"

// Screen size related parameters.
#include "doomdef.h"

// Include the refresh/render data structs.
#include "r_data.h"


//
// Separate header file for each module.
//
#include "r_main.h"
#include "r_bsp.h"
#include "r_segs.h"
#include "r_plane.h"
#include "r_data.h"
#include "r_things.h"
#include "r_draw.h"

#endif		// __R_LOCAL__
