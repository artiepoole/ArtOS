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

#include "EventQueue.h"

#include <Scheduler.h>

#include "ports.h"
#include "key_maps.h"
#include "logging.h"

/* US Keyboard Layout. This is a scancode table
*  used to layout a standard US keyboard. I have left some
*  comments in to give you an idea of what key is what, even
*  though I set it's array index to 0. */

size_t DEF_MAX_EVENTS = 32;
/* Handles the keyboard interrupt */
void keyboardHandler()
{
    EventQueue* queue = Scheduler::getCurrentProcessEventQueue();

    /* Read from the keyboard's data buffer */
    if (queue == NULL) return;

    if (const u32 scancode = inb(KEYB_DATA); scancode & 0x80) // key down event
    {
        const auto ku = event_t{KEY_UP, event_data_t{scancode - 0x80, 0}};
        queue->addEvent(ku);
        /* You can use this one to see if the user released the
        *  shift, alt, or control keys... */
    }
    else // key release event
    {
        const auto kd = event_t{KEY_DOWN, event_data_t{scancode, 0}};
        queue->addEvent(kd);
    }
}

EventQueue::EventQueue()
{
    LOG("Initialising EventQueue");
    _unread_counter = 0;
    _write_index = 0;
    _read_index = 0;
    _event_queue = new event_t[DEF_MAX_EVENTS];
    max_len = DEF_MAX_EVENTS;
    if (_event_queue == nullptr) return;
    for (size_t i = 0; i < DEF_MAX_EVENTS; i++)
    {
        _event_queue[i] = event_t{NULL_EVENT, event_data_t{0, 0}};
    }
    LOG("EventQueue initialised");
}

EventQueue::~EventQueue()
{
    delete[] _event_queue;
}

EventQueue::EventQueue(const size_t max_items)
{
    LOG("Initialising EventQueue");
    _unread_counter = 0;
    _write_index = 0;
    _read_index = 0;

    _event_queue = new event_t[max_items];
    max_len = max_items;
    if (_event_queue == nullptr) return;
    for (size_t i = 0; i < max_items; i++)
    {
        _event_queue[i] = event_t{NULL_EVENT, event_data_t{0, 0}};
    }
    LOG("EventQueue initialised");
}

void EventQueue::addEvent(const event_t& event)
{
    _event_queue[_write_index] = event;
    _write_index = (_write_index + 1) % max_len;
    _unread_counter++;
}


bool EventQueue::pendingEvents() const
{
    return _unread_counter > 0;
}


event_t EventQueue::getEvent()
{
    if (_unread_counter == 0)
    {
        WRITE("Tried to get read event ahead of event queue. Returning NONE event");
        return event_t{NULL_EVENT, event_data_t{0, 0}};
    }

    const auto event_out = _event_queue[_read_index];
    _unread_counter--;
    _read_index = (_read_index + 1) % max_len;
    return event_out;
}
