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


#ifndef __D_TICCMD__
#define __D_TICCMD__

#include "doomtype.h"


// The data sampled per tick (single player)
// and transmitted to other peers (multiplayer).
// Mainly movements/button commands per game tick,
// plus a checksum for internal state consistency.

typedef struct
{
    signed char forwardmove; // *2048 for move
    signed char sidemove; // *2048 for move
    short angleturn; // <<16 for angle delta
    byte chatchar;
    byte buttons;
    // villsa [STRIFE] according to the asm,
    // consistancy is a short, not a byte
    byte consistancy; // checks for net game

    // villsa - Strife specific:

    byte buttons2;
    int inventory;

    // Heretic/Hexen specific:

    byte lookfly; // look/fly up/down/centering
    byte arti; // artitype_t to use
} ticcmd_t;


#endif
