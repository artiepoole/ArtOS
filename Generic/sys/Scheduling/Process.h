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
// Created by artypoole on 21/11/24.
//

#ifndef PROCESS_H
#define PROCESS_H

#include "types.h"
#include "CPU.h"
#include "limits.h"


class EventQueue;
class PagingTableUser;

#ifdef __cplusplus
//https://en.wikipedia.org/wiki/Task_state_segment
struct Process
{
    Process();
    void reset();
    void start(size_t parent_id, const cpu_registers_t& new_context, void* new_stack, const char* new_name, const char* abs_path, bool is_user);
    ~Process();

    enum State_t
    {
        STATE_DEAD,
        STATE_EXITED,
        STATE_SLEEPING,
        STATE_PARKED,
        STATE_READY,
    };


    // Used to scale execution duration.
    enum Priority_t
    {
        PRIORITY_LOW = 1,
        PRIORITY_NORMAL = 10,
        PRIORITY_HIGH = 100,
    };

    bool isParked() { return state == STATE_PARKED; }
    bool isDead() { return state == STATE_DEAD; }

    u32 parent_pid;
    State_t state;
    Priority_t priority;
    cpu_registers_t context;
    //    u32 base_vaddr;
    char name[MAX_FILENAME_BUF]; //this can be stored in an equivalent of proc?
    void* stack;
    EventQueue* eventQueue;
    bool user;
    PagingTableUser* paging_table;
    uintptr_t cr3_val;
    u64 last_executed;
    char pwd[MAX_PATH_BUF]; // max path length is 256
};


#endif

#endif //PROCESS_H
