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
#include <IOAPIC.h>
#include <../../../ArtOS_lib/kernel.h>
#include <LocalAPIC.h>
#include <logging.h>
#include <PagingTableUser.h>
#include <SMBIOS.h>
#include <syscall.h>
#include <TSC.h>
#include <Memory/PagingTable.h>

#include "CPUID.h"
#include "memory.h"
#include "LinkedList.h"
#include "EventQueue.h"
#include "Process.h"


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


extern u8 kernel_stack_top;
extern u8 kernel_stack_bottom;

void idle_task()
{
    LOG("Starting idle task");
    while (true);
}

int kyield()
{
    // TODO: this should mark the thread as yielded somehow.
    constexpr u8 irq = LAPIC_IRQ + 32; // Example dynamic value
    __asm__ __volatile__("int %0" :: "i"(irq));
    return 0;
}


Scheduler::Scheduler(LocalAPIC* timer, EventQueue* kernel_queue)
{
    // Store current state in process[0]
    // Set up first Lapic one shot
    disable_interrupts();
    scheduler_instance = this;
    lapic_timer = timer;
    const auto nm = "scheduler";
    art_string::strncpy(processes[0].name, nm, MIN(32, art_string::strlen(nm)));
    processes[0].state = Process::STATE_PARKED;
    processes[0].eventQueue = kernel_queue;
    processes[0].stack = &kernel_stack_top;
    execution_counter = TSC_get_ticks();
    create_idle_task();
    // execf(, main_func, name, false);
}

Scheduler::~Scheduler()
{
    scheduler_instance = nullptr;
}

Scheduler& Scheduler::get()
{
    return *scheduler_instance;
}

/*
 * Create a new page table and directory for the user process (new CR3).
 * Allocate physical memory for the process’s stack, code, and data.
 * Map those physical pages into the user-space virtual addresses in the new page table.
 * Temporarily map those physical pages into the current kernel address space, so you can write to them.
 * Write values (like initial stack contents, arguments, etc.) via the kernel mapping.
 * Later, when the process runs, load its CR3, set its initial stack pointer, and iret.
*/
// Create new process. This can only be user mode if the code and data for the function are contained in user-accessible pages
void Scheduler::execf(cpu_registers_t* r, u32 func, uintptr_t name_loc, const bool user)
{
    auto name = reinterpret_cast<const char*>(name_loc);
    // Should copy stack contents but I am not sure if they are already ruined by this.R=
    size_t next_process_id = getNextFreeProcessID();
    const size_t parent_process_id = current_process_id;
    if (next_process_id + 1 >= max_processes) return; // TODO: Error

    auto* proc = &processes[next_process_id];


    cpu_registers_t context{};
    proc->user = user;
    void* proc_stack;
    void* stack_top;
    if (user)
    {
        proc->paging_table = new PagingTableUser();
        // TOOD: this needs to be replaced so that a malloc call can be done using flags instead.
        // TODO: the stack must be part of the user space memory map so this has to be remapped!
        proc_stack = art_alloc(stack_size, stack_alignment);
        stack_top = static_cast<u8*>(proc_stack) + stack_size;
    }
    else
    {
        // proc_stack = art_alloc(stack_size, stack_alignment);
        // stack_top = static_cast<u8*>(proc_stack) + stack_size;
        // proc_stack = &kernel_stack_bottom;
        stack_top = &kernel_stack_top;
    }
    LOG("Starting Process: ", name, " PID: ", next_process_id);
    context.esp = reinterpret_cast<u32>(stack_top);

    if (user)
    {
        context.cs = user_cs_offset | RPL_USER;
        context.ds = user_ds_offset | RPL_USER;
        context.es = user_ds_offset | RPL_USER;
        context.fs = user_ds_offset | RPL_USER;
        context.gs = user_ds_offset | RPL_USER;
        context.ss = user_ds_offset | RPL_USER;
    }
    else
    {
        context.cs = kernel_cs_offset;
        context.ds = kernel_ds_offset;
        context.es = kernel_ds_offset;
        context.fs = kernel_ds_offset;
        context.gs = kernel_ds_offset;
        context.ss = kernel_ds_offset;
    }
    context.eip = reinterpret_cast<u32>(func);
    context.eflags = default_eflags;


    proc->start(parent_process_id, context, proc_stack, name, user);

    processes[parent_process_id].state = Process::STATE_PARKED;
    schedule(r);
}


// TODO (URGENT): This should swap paging directory. See: https://wiki.osdev.org/Kernel_Multitasking#Kernel_Stack_Per_Task
// Specifically the `cmp eax,ecx` lines.
void Scheduler::switch_process(cpu_registers_t* const r, size_t new_PID)
{
    // This should just pop the stack and push to it to replace next process. If process is 0 idk what to do.
    if (new_PID == 0)
    {
        new_PID = 1;
    }
    // TODO: is r here editable to replace data on the stack?!
    // if (processes[current_process_id].state != Process::STATE_DEAD && processes[current_process_id].state != Process::STATE_EXITED)
    store_current_context(r, current_process_id);
    current_process_id = new_PID;
    const auto priority = processes[current_process_id].priority;
    // LOG("Switching to ", processes[current_process_id].name);
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

// Returns the id of the process which is parked or alive which has the highest id.
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
            if (processes[i].user)
            {
                art_free(processes[i].stack);
            }
            processes[i].reset(); // cleans up event queue.
            if (i == highest_assigned_pid)
            {
                highest_assigned_pid = getMaxAliveProcessID();
            }
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

// bool Scheduler::isCurrentProcessUser()
// {
//     return processes[current_process_id].user;
// }

// bool Scheduler::isProcessUser(size_t PID)
// {
//     return processes[PID].user;
// }

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
        if (processes[pid].state == Process::STATE_SLEEPING)
        {
            processes[pid].state = Process::STATE_READY;
        }
    }
}

size_t get_oldest_process()
{
    size_t ret_id = 0;
    size_t lowest = -1;
    for (size_t i = 2; i <= highest_assigned_pid; i++)
    {
        if (processes[i].state != Process::STATE_READY) continue;
        if (processes[i].last_executed < lowest)
        {
            ret_id = i;
            lowest = processes[i].last_executed;
        }
    }
    if (ret_id == 0) return 1;
    return ret_id;
}

size_t Scheduler::getNextProcessID()
{
    const size_t next = get_oldest_process(); // dec and return split here for debugging purposes.
    return next;
}

void Scheduler::start_oneshot(u32 time_ms)
{
    lapic_timer->start_timer(time_ms);
}

//TODO: refactor. this no longer converts
void Scheduler::store_current_context(cpu_registers_t* r, const size_t PID)
{
    art_string::memcpy(&processes[PID].context, r, sizeof(cpu_registers_t));
}

void Scheduler::set_current_context(cpu_registers_t* r, size_t PID)
{
    art_string::memcpy(r, &processes[PID].context, sizeof(cpu_registers_t));
    // r->esp = reinterpret_cast<u32>(processes[PID].stack);

    // TODO: This contained logic here may be incorrect.
    // When going from CPL3 to CPL3, I am not sure that overwriting user_esp is correct.
    // It could be better to only memcpy 8 fewer bytes to preserve useresp and ss?
    // we only really need to store the new EIP value or the new cs, eflags, user_esp and new ss for a user mode switch.

    if (processes[PID].user)
    {
        // auto addr =  | 0xFFF;
        // __asm__ volatile ("mov %0, %%cr3" : : "r"(addr));
        set_cr3(processes[PID].paging_table->get_page_table_addr());
    }
    else
    {
        auto addr = get_kernal_page_dir() | 0xFFF;
        __asm__ volatile ("mov %0, %%cr3" : : "r"(addr));
        // set_cr3(get_kernal_page_dir());
    }
}

void Scheduler::sleep_ms(const u32 ms)
{
    sleep_timers.append(sleep_timer_t{current_process_id, ms});
    processes[current_process_id].state = Process::STATE_SLEEPING;
    start_oneshot(MIN(ms, context_switch_period_ms));
    kyield();
}

// When called from interrupt, the state is stored at this pointer loc, r.
void Scheduler::schedule(cpu_registers_t* const r)
{
    processes[current_process_id].last_executed = execution_counter;
    clean_up_exited_threads();
    handle_expired_timers();
    const size_t next_id = getNextProcessID();
    switch_process(r, next_id);
}


void LAPIC_handler(cpu_registers_t* const r)
{
    Scheduler::schedule(r);
    // TODO: implement scheduler
}

// Exit is called by the program to tell the OS it is done.
void Scheduler::exit(cpu_registers_t* const r)
{
    LOG("Exiting ", processes[current_process_id].name, " PID: ", current_process_id, " with status: ", (u32)r->ebx);
    processes[current_process_id].state = Process::STATE_EXITED;
    auto parent_id = processes[current_process_id].parent_pid;
    if (processes[parent_id].state == Process::STATE_PARKED)
    {
        processes[parent_id].state = Process::STATE_READY;
    }
    size_t next = getNextProcessID();
    LOG("Post-exit process ID:", next);

    switch_process(r, next);
}

// Kill is supposed to send a command to the process to tell it to exit.
void Scheduler::kill(size_t target_pid)
{
    processes[target_pid].state = Process::STATE_EXITED;
    auto parent_id = processes[current_process_id].parent_pid;
    if (processes[parent_id].state == Process::STATE_PARKED)
    {
        processes[parent_id].state = Process::STATE_READY;
    }
    // ???? what do here? I cannot switch because then it never returns?
    // switch_process(r, getNextProcessID());
    // kyield();
}

void Scheduler::create_idle_task()
{
    const size_t next_process_id = getNextFreeProcessID();
    if (next_process_id + 1 >= max_processes) return; // TODO: Error
    auto* proc = &processes[next_process_id];
    cpu_registers_t context{};

    context.cs = kernel_cs_offset;
    context.ds = kernel_ds_offset;
    context.es = kernel_ds_offset;
    context.fs = kernel_ds_offset;
    context.gs = kernel_ds_offset;
    context.ss = kernel_ds_offset;

    context.eip = reinterpret_cast<u32>(&idle_task);
    context.eflags = default_eflags;

    proc->state = Process::STATE_READY;
    proc->parent_pid = 0;
    proc->user = false;
    // void* proc_stack = art_alloc(stack_size, stack_alignment);
    // void* stack_top = static_cast<u8*>(proc_stack) + stack_size;
    proc->stack = &kernel_stack_top;
    proc->last_executed = 0;
    proc->priority = Process::PRIORITY_LOW;
    const auto nm = "idle_task";
    art_string::strncpy(proc->name, nm, MIN(32, art_string::strlen(nm)));
    proc->context = context;
    proc->eventQueue = new EventQueue();
}
