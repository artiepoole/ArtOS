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


#ifndef __R_DATA__
#define __R_DATA__

#include "r_defs.h"
#include "r_state.h"


// Retrieve column data for span blitting.
byte*
R_GetColumn
(int tex,
 int col);


// I/O, setting up the stuff.
void R_InitData(void);
void R_PrecacheLevel(void);


// Retrieval.
// Floor/ceiling opaque texture tiles,
// lookup by name. For animation?
int R_FlatNumForName(char* name);


// Called by P_Ticker for switches and animations,
// returns the texture number for the texture name.
int R_TextureNumForName(char* name);
int R_CheckTextureNumForName(char* name);

#endif
