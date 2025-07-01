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

#ifndef SCHEDULER_H
#define SCHEDULER_H

#include <LocalAPIC.h>

#include  "types.h"

inline size_t stack_size = 1024 * 1024; // 1MB stack default. Probably not enough.
constexpr size_t max_processes = 255;

class EventQueue;
class PagingTableUser;

class Scheduler {
public:
    Scheduler(LocalAPIC *timer, EventQueue *kernel_queue);

    ~Scheduler();

    static Scheduler &get();

    // void start(size_t PID);
    // static void switch_process(size_t new_PID);
    static void switch_process(cpu_registers_t *r, size_t new_PID);

    static size_t getNextFreeProcessID();

    static size_t getMaxAliveProcessID();

    // Only takes void foo() types atm. No support for input variables
    static void execf(cpu_registers_t *r, uintptr_t func, uintptr_t name, bool user);

    // static void fork();
    static void exit(cpu_registers_t *r);

    static void kill(size_t target_pid);

    static void create_idle_task();

    static void execute_from_paging_table(PagingTableUser *PTU, const char *name_loc, uintptr_t entry_point,
                                          uintptr_t stack_vaddr, uintptr_t stack_size);

    PagingTableUser *getCurrentPagingTable();

    static void clean_up_exited_threads();

    static size_t getCurrentProcessID();

    static EventQueue *getCurrentProcessEventQueue();

    static uintptr_t getCurrentProcessPagingDirectory();

    // static bool isCurrentProcessUser();
    // static bool isProcessUser(size_t PID);
    static size_t getNextProcessID();

    static void start_oneshot(u32 time_ms);

    // static void store_current_context(size_t PID);
    static void store_current_context(cpu_registers_t *r, size_t PID);

    static void set_current_context(cpu_registers_t *r, size_t PID);

    static void schedule(cpu_registers_t *r);

    // static void schedule();

    static void sleep_ms(cpu_registers_t *r);
};


#endif //SCHEDULER_H
