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


// Needed for FRACUNIT.
#include "m_fixed.h"

// Needed for Flat retrieval.
#include "r_data.h"


#include "r_sky.h"

//
// sky mapping
//
int skyflatnum;
int skytexture;
int skytexturemid;


//
// R_InitSkyMap
// Called whenever the view size changes.
//
void R_InitSkyMap(void)
{
    // skyflatnum = R_FlatNumForName ( SKYFLATNAME );
    skytexturemid = 100 * FRACUNIT;
}
