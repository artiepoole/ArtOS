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


#ifndef __M_ARGV__
#define __M_ARGV__

#include "doomtype.h"

//
// MISC
//
extern int myargc;
extern char** myargv;

// Returns the position of the given parameter
// in the arg list (0 if not found).
int M_CheckParm(char* check);

// Same as M_CheckParm, but checks that num_args arguments are available
// following the specified argument.
int M_CheckParmWithArgs(char* check, int num_args);

void M_FindResponseFile(void);

// Parameter has been specified?

boolean M_ParmExists(char* check);

// Get name of executable used to run this program:

char* M_GetExecutableName(void);

#endif
