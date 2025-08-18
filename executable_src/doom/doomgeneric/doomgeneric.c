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

#include "stdio.h"
#include "m_argv.h"

#include "doomgeneric.h"
#include "d_main.h"

pixel_t* DG_ScreenBuffer = NULL;
boolean doom_is_running;

void M_FindResponseFile(void);
void D_DoomMain(void);


void doomgeneric_Create(int argc, char** argv)
{
    // save arguments
    myargc = argc;
    myargv = argv;
    doom_is_running = true;

    // M_FindResponseFile();

    DG_ScreenBuffer = malloc(DOOMGENERIC_RESX * DOOMGENERIC_RESY * 4);

    DG_Init();

    D_DoomMain();
}
