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
// Created by artypoole on 06/01/25.
//

#ifndef EVENT_H
#define EVENT_H

#include "types.h"

enum EVENT_TYPE
{
    NULL_EVENT = 0,
    TICK = 1,
    DRAW_CALL = 2,
    KEY_DOWN = 3,
    KEY_UP = 4,
    MOUSE = 5,
};

struct event_data_t
{
    u32 lower_data;
    u32 upper_data;
};

struct event_t
{
    EVENT_TYPE type;
    event_data_t data;
};

#endif //EVENT_H
