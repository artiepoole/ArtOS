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
#include "Process.h"

#include <cmp_int.h>
#include <string.h>

#include "EventQueue.h"

Process::Process()
{
    state = STATE_DEAD;
    parent_pid = -1;
    priority = PRIORITY_NORMAL;
    last_executed = 0;
    context = cpu_registers_t{};
    stack = NULL;
    name[0] = '\0';
    eventQueue = NULL;
    user = false;
}

void Process::reset()
{
    state = STATE_DEAD;
    parent_pid = -1;
    priority = PRIORITY_NORMAL;
    last_executed = 0;
    context = cpu_registers_t{};
    stack = NULL;
    name[0] = '\0';
    if (eventQueue != NULL)
    {
        delete eventQueue;
        eventQueue = NULL;
    }
    user = false;
}


void Process::start(const size_t parent_id, const cpu_registers_t& new_context, void* new_stack, const char* new_name, const bool is_user)
{
    parent_pid = parent_id;
    state = STATE_READY;
    priority = PRIORITY_NORMAL;
    last_executed = 0;
    context = new_context;
    if (is_user)
    {
        stack = new_stack;
    }
    else
    {
        stack = NULL;
    }
    eventQueue = new EventQueue();
    strncpy(name, new_name, MIN(32, strlen(new_name)));
    user = is_user;
}

Process::~Process()
{
    if (eventQueue != NULL)
    {
        delete eventQueue;
    }
}
