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
// Created by artypoole on 02/01/25.
//
#include "Scheduler.h"

#include <cmp_int.h>
#include <GDT.h>
#include <IDT.h>
#include <LocalAPIC.h>
#include <logging.h>
#include <SMBIOS.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <TSC.h>

#include "CPUID.h"
#include "kernel.h"
#include "memory.h"
#include "LinkedList.h"
#include "EventQueue.h"


struct sleep_timer_t
{
    size_t pid;
    i64 counter;
};


size_t stack_alignment = 16;

u32 default_eflags = 0x206;
Scheduler* scheduler_instance = nullptr;
u32 execution_counter = 0;
size_t current_process_id = 0;
size_t highest_assigned_pid = 0;
// size_t next_process_id = 1;
size_t context_switch_period_ms = 1;
Process processes[max_processes];
LocalAPIC* lapic_timer = nullptr;


LinkedList<sleep_timer_t> sleep_timers;


Scheduler::Scheduler(void (*main_func)(), char* name, LocalAPIC* timer, EventQueue* kernel_queue)
{
    // Store current state in process[0]
    // Set up first Lapic one shot

    scheduler_instance = this;
    lapic_timer = timer;
    const auto nm = "scheduler";
    strncpy(processes[0].name, nm, MIN(32, strlen(nm)));
    processes[0].state = Process::STATE_PARKED;
    processes[0].eventQueue = kernel_queue;
    execution_counter = TSC_get_ticks();
    execf(main_func, name);
}

Scheduler::~Scheduler()
{
    scheduler_instance = nullptr;
}

Scheduler& Scheduler::get()
{
    return *scheduler_instance;
}

// Create new process
void Scheduler::execf(void (*func)(), const char* name) // if user mode then the CS and DS should be different
{
    size_t next_process_id = getNextFreeProcessID();
    if (next_process_id + 1 >= max_processes) return; // TODO: Error
    void* proc_stack = aligned_malloc(stack_size, stack_alignment);
    void* stack_top = static_cast<u8*>(proc_stack) + stack_size;
    LOG("Starting Process: ", name, " PID: ", next_process_id);
    cpu_registers_t context{};
    context.esp = reinterpret_cast<u32>(stack_top);
    context.cs = kernel_cs_offset;
    context.ds = kernel_ds_offset;
    context.es = kernel_ds_offset;
    context.fs = kernel_ds_offset;
    context.gs = kernel_ds_offset;
    context.ss = kernel_ds_offset;
    context.eip = reinterpret_cast<u32>(func);
    context.eflags = default_eflags;
    auto* proc = &processes[next_process_id];
    proc->start(current_process_id, context, proc_stack, name);
    processes[current_process_id].state = Process::STATE_PARKED;
    kyield();
}


void Scheduler::switch_process(cpu_registers_t* const r, const size_t new_PID)
{
    // This should just pop the stack and push to it to replace next process. If process is 0 idk what to do.
    if (new_PID == 0)
    {
        start_oneshot(context_switch_period_ms);
        kyield();
    }
    // TODO: is r here editable to replace data on the stack?!
    if (processes[current_process_id].state != Process::STATE_DEAD) convert_current_context(r, current_process_id);
    current_process_id = new_PID;
    const auto priority = processes[current_process_id].priority;
    start_oneshot(context_switch_period_ms * priority);
    execution_counter = TSC_get_ticks();
    set_current_context(r, current_process_id);
}

size_t Scheduler::getNextFreeProcessID()
{
    size_t ret_id = 0;
    while (processes[ret_id].state != Process::STATE_DEAD) { ret_id++; }
    if (ret_id >= max_processes) kyield(); // TODO: THROW!
    if (ret_id > highest_assigned_pid) highest_assigned_pid = ret_id;
    return ret_id;
}

size_t Scheduler::getMaxAliveProcessID()
{
    size_t ret_id = 0;
    for (size_t i = 0; i <= highest_assigned_pid; i++)
    {
        if (processes[i].state != Process::STATE_DEAD || processes[i].state != Process::STATE_EXITED) ret_id = i;
    }
    if (highest_assigned_pid < ret_id) highest_assigned_pid = ret_id;
    return ret_id;
}

// size_t Scheduler::getCurrentProcessID()
// {
// }


void Scheduler::clean_up_exited_threads()
{
    for (size_t i = 0; i <= highest_assigned_pid; i++)
    {
        if (processes[i].state == Process::STATE_EXITED)
        {
            aligned_free(processes[i].stack);
            processes[i].reset(); // cleans up event queue.
        }
    }
}

size_t Scheduler::getCurrentProcessID()
{
    return current_process_id;
}

EventQueue* Scheduler::getCurrentProcessEventQueue()
{
    return processes[current_process_id].eventQueue;
}


// files.find_if([filename](ArtFile f) { return strcmp(f.get_name(), filename) == 0; });
// iterate([device](ArtDirectory* dir) { device->populate_directory_recursive(dir); });
void handle_expired_timers()
{
    u32 elapsed_ms = (TSC_get_ticks() - execution_counter) / (cpuid_get_TSC_frequency() / 1000);
    sleep_timers.iterate([elapsed_ms](sleep_timer_t* t) { t->counter -= elapsed_ms; });

    while (true)
    {
        sleep_timer_t* timer = sleep_timers.find_if([](sleep_timer_t t) { return t.counter < 0; });
        if (timer == nullptr) return;
        size_t pid = timer->pid;
        sleep_timers.remove(timer);
        if (processes[pid].state == Process::STATE_PARKED)
        {
            processes[pid].state = Process::STATE_READY;
        }
    }
}

size_t get_oldest_process()
{
    size_t ret_id = 0;
    size_t lowest = -1;
    for (size_t i = 0; i <= highest_assigned_pid; i++)
    {
        if (processes[i].state != Process::STATE_READY) continue;
        if (processes[i].last_executed < lowest)
        {
            ret_id = i;
            lowest = processes[i].last_executed;
        }
    }

    return ret_id;
}

size_t Scheduler::getNextProcessID()
{
    clean_up_exited_threads();
    handle_expired_timers();
    size_t next = get_oldest_process();
    return next;
}

void Scheduler::start_oneshot(u32 time_ms)
{
    lapic_timer->start_timer(time_ms);
}


void Scheduler::convert_current_context(cpu_registers_t* r, const size_t PID)
{
    memcpy(&processes[PID].context, r, sizeof(cpu_registers_t));
}

void Scheduler::set_current_context(cpu_registers_t* r, size_t PID)
{
    memcpy(r, &processes[PID].context, sizeof(cpu_registers_t));
}

void Scheduler::sleep_ms(const u32 ms)
{
    sleep_timers.append(sleep_timer_t{current_process_id, ms});
    processes[current_process_id].state = Process::STATE_PARKED;
    start_oneshot(MIN(ms, context_switch_period_ms));
    kyield();
}

// When called from interrupt, the state is stored at this pointer loc, r.
void Scheduler::schedule(cpu_registers_t* const r)
{
    const size_t next_id = getNextProcessID();
    switch_process(r, next_id);
}


void LAPIC_handler(cpu_registers_t* const r)
{
    Scheduler::schedule(r);
    // TODO: implement scheduler
}

int kyield()
{
    // TODO: this should mark the thread as yielded somehow.
    constexpr u8 irq = LAPIC_IRQ + 32; // Example dynamic value
    __asm__ __volatile__("int %0" :: "i"(irq));
    return 0;
}


void Scheduler::exit(int status)
{
    LOG("Exiting ", processes[current_process_id].name, " PID: ", current_process_id, " with status: ", status);
    processes[current_process_id].state = Process::STATE_EXITED;
    auto parent_id = processes[current_process_id].parent_pid;
    if (processes[parent_id].state == Process::STATE_PARKED)
    {
        processes[parent_id].state = Process::STATE_READY;
        // no return
    }
    kyield();
}
