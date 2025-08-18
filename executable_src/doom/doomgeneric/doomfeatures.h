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

#ifndef DOOM_FEATURES_H
#define DOOM_FEATURES_H

// Enables wad merging (the '-merge' command line parameter)

#undef FEATURE_WAD_MERGE

// Enables dehacked support ('-deh')

#undef FEATURE_DEHACKED

// Enables multiplayer support (network games)

#undef FEATURE_MULTIPLAYER

// Enables sound output

//#undef FEATURE_SOUND

#endif /* #ifndef DOOM_FEATURES_H */
