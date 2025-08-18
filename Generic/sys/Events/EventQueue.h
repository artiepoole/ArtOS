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
// Created by artypoole on 06/07/24.
//

#ifndef KEYBOARD_H
#define KEYBOARD_H

#include "types.h"
#include "event.h"


extern const char key_map[128];
extern const char shift_map[128];


void keyboardHandler();

class EventQueue
{
public:
    EventQueue();
    ~EventQueue();
    explicit EventQueue(size_t max_items);
    void addEvent(const event_t& event);
    [[nodiscard]] bool pendingEvents() const;
    event_t getEvent();

private:
    size_t max_len = 0;
    event_t* _event_queue;
    size_t _unread_counter;
    size_t _write_index;
    size_t _read_index;
};


#endif //KEYBOARD_H
