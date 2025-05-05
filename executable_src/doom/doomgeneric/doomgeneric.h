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

#ifndef DOOM_GENERIC
#define DOOM_GENERIC

#include "stdlib.h"
#include <stdint.h>
#include "d_main.h"

#ifndef DOOMGENERIC_RESX
#if FORLAPTOP
        #define DOOMGENERIC_RESX 1920
#else
#define DOOMGENERIC_RESX 1280
#endif
#endif  // DOOMGENERIC_RESX

#ifndef DOOMGENERIC_RESY
#if FORLAPTOP
        #define DOOMGENERIC_RESY 1080
#else
#define DOOMGENERIC_RESY 960
#endif
#endif  // DOOMGENERIC_RESY

#include "doomtypes.h"
#ifdef CMAP256

typedef uint8_t pixel_t;

#else  // CMAP256

typedef u32 pixel_t;

#endif  // CMAP256


extern pixel_t* DG_ScreenBuffer;
extern boolean doom_is_running;
void doomgeneric_Create(int argc, char** argv);
boolean doomgeneric_Tick();


//Implement below functions for your platform

void DG_Init();
void DG_DrawFrame();
void DG_SleepMs(uint32_t ms);
uint32_t DG_GetTicksMs();
int DG_GetKey(int* pressed, unsigned char* key);
void DG_SetWindowTitle(const char* title);


int run_doom();
[[noreturn]] void run_doom_noret();

#endif //DOOM_GENERIC
