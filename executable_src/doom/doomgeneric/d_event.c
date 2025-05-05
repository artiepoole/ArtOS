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

#include <stdlib.h>
#include "d_event.h"

#define MAXEVENTS 64

static doom_event_t events[MAXEVENTS];
static int eventhead;
static int eventtail;

//
// D_PostEvent
// Called by the I/O functions when input is detected
//
void D_PostEvent(doom_event_t* ev)
{
    events[eventhead] = *ev;
    eventhead = (eventhead + 1) % MAXEVENTS;
}

// Read an event from the queue.

doom_event_t* D_PopEvent(void)
{
    doom_event_t* result;

    // No more events waiting.

    if (eventtail == eventhead)
    {
        return NULL;
    }

    result = &events[eventtail];

    // Advance to the next event in the queue.

    eventtail = (eventtail + 1) % MAXEVENTS;

    return result;
}
