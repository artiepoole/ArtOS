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
#include <Files.h>
#include <GDT.h>
#include <IDT.h>
#include <IOAPIC.h>
#include <../../../ArtOS_lib/kernel.h>
#include <LocalAPIC.h>
#include <logging.h>
#include <PagingTableKernel.h>
#include <PagingTableUser.h>
#include <SMBIOS.h>
#include <string.h>
#include <syscall.h>
#include <TSC.h>
#include <Memory/PagingTable.h>

#include "CPUID.h"
#include "memory.h"
#include "LinkedList.h"
#include "EventQueue.h"
#include "Process.h"

#define LOG_IDLE false
#ifdef NDEBUG
    #define CONTEXT_SWITCH_PERIOD_US 1000  // Release
#else
#define CONTEXT_SWITCH_PERIOD_US 10000 // Debug or other
#endif

size_t context_switch_period_us = CONTEXT_SWITCH_PERIOD_US;

struct sleep_timer_t
{
    size_t pid;
    long double counter;
};


size_t stack_alignment = 16;

u32 default_eflags = 0x206;
Scheduler *scheduler_instance = nullptr;
u64 execution_counter = 0;
size_t current_process_id = 0;
size_t highest_assigned_pid = 0;
// size_t next_process_id = 1;
Process processes[max_processes];
LocalAPIC *lapic_timer = nullptr;


LinkedList<sleep_timer_t> sleep_timers;


extern u8 kernel_stack_top;
extern u8 kernel_stack_bottom;

void idle_task() {
    while (true) {
    };
}


Scheduler::Scheduler(LocalAPIC *timer, EventQueue *kernel_queue) {
    // Store current state in process[0]
    // Set up first Lapic one shot
    // disable_interrupts();
#if ENABLE_SERIAL_LOGGING
    get_serial().log("context_switch_period_us: ", context_switch_period_us);
#endif
    execution_counter = TSC_get_ticks();
    art_string::memset(processes, 0, sizeof(Process) * max_processes);
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

Scheduler::~Scheduler() {
    scheduler_instance = nullptr;
}

Scheduler &Scheduler::get() {
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
void Scheduler::execf(cpu_registers_t *r, u32 func, uintptr_t name_loc, const bool user) {
    auto name = reinterpret_cast<const char *>(name_loc);
    // Should copy stack contents but I am not sure if they are already ruined by this.R=
    size_t next_process_id = getNextFreeProcessID();
    const size_t parent_process_id = current_process_id;
    if (next_process_id + 1 >= max_processes) return; // TODO: Error

    auto *proc = &processes[next_process_id];


    cpu_registers_t context{};
    proc->user = user;
    void *proc_stack;
    void *stack_top;
    if (user) {
        proc->paging_table = new PagingTableUser();
        proc->cr3_val = proc->paging_table->get_phys_addr_of_page_dir();
        // TOOD: this needs to be replaced so that a malloc call can be done using flags instead.
        // TODO: the stack must be part of the user space memory map so this has to be remapped!
        proc_stack = art_alloc(stack_size, stack_alignment);
        stack_top = static_cast<u8 *>(proc_stack) + stack_size;
    } else {
        // proc_stack = art_alloc(stack_size, stack_alignment);
        // stack_top = static_cast<u8*>(proc_stack) + stack_size;
        // proc_stack = &kernel_stack_bottom;
        proc->cr3_val = kernel_pages().get_phys_addr_of_page_dir();
        stack_top = &kernel_stack_top;
    }
    LOG("Starting Process: ", name, " PID: ", next_process_id);
    context.esp = reinterpret_cast<u32>(stack_top);

    if (user) {
        context.cs = user_cs_offset | RPL_USER;
        context.ds = user_ds_offset | RPL_USER;
        context.es = user_ds_offset | RPL_USER;
        context.fs = user_ds_offset | RPL_USER;
        context.gs = user_ds_offset | RPL_USER;
        context.ss = user_ds_offset | RPL_USER;
    } else {
        context.cs = kernel_cs_offset;
        context.ds = kernel_ds_offset;
        context.es = kernel_ds_offset;
        context.fs = kernel_ds_offset;
        context.gs = kernel_ds_offset;
        context.ss = kernel_ds_offset;
    }
    context.eip = func;
    context.eflags = default_eflags;

    proc->last_executed = TSC_get_ticks();
    proc->start(parent_process_id, context, proc_stack, name, user);

    processes[parent_process_id].state = Process::STATE_PARKED;
    schedule(r);
}


// TODO (URGENT): This should swap paging directory. See: https://wiki.osdev.org/Kernel_Multitasking#Kernel_Stack_Per_Task
// Specifically the `cmp eax,ecx` lines.
void Scheduler::switch_process(cpu_registers_t *const r, size_t new_PID) {
    // This should just pop the stack and push to it to replace next process. If process is 0 idk what to do.
    if (new_PID == 0) {
        new_PID = 1;
    }

#if ENABLE_SERIAL_LOGGING and LOG_IDLE
    if (new_PID == 1) {
        get_serial().log("switching to idle task");
    }
#endif

    // TODO: is r here editable to replace data on the stack?!
    // if (processes[current_process_id].state != Process::STATE_DEAD && processes[current_process_id].state != Process::STATE_EXITED)
    store_current_context(r, current_process_id);
    processes[current_process_id].last_executed = execution_counter;
    current_process_id = new_PID;
    const auto priority = processes[current_process_id].priority;
    // LOG("Switching to ", processes[current_process_id].name);
    start_oneshot(context_switch_period_us * priority);
    set_current_context(r, current_process_id);
}

size_t Scheduler::getNextFreeProcessID() {
    size_t ret_id = 0;
    while (processes[ret_id].state != Process::STATE_DEAD) { ret_id++; }
    if (ret_id > highest_assigned_pid) highest_assigned_pid = ret_id;
    return ret_id;
}

// Returns the id of the process which is parked or alive which has the highest id.
size_t Scheduler::getMaxAliveProcessID() {
    size_t ret_id = 0;
    for (size_t i = 0; i <= highest_assigned_pid; i++) {
        if (processes[i].state != Process::STATE_DEAD || processes[i].state != Process::STATE_EXITED) ret_id = i;
    }
    if (highest_assigned_pid < ret_id) highest_assigned_pid = ret_id;
    return ret_id;
}

// size_t Scheduler::getCurrentProcessID()
// {
// }


void Scheduler::clean_up_exited_threads() {
    for (size_t i = 0; i <= highest_assigned_pid; i++) {
        if (processes[i].state == Process::STATE_EXITED) {
            if (processes[i].user) {
                art_free(processes[i].stack);
            }
            processes[i].reset(); // cleans up event queue.
            if (i == highest_assigned_pid) {
                highest_assigned_pid = getMaxAliveProcessID();
            }
        }
    }
}

void Scheduler::check_finished_io_read() {
    for (size_t i = 0; i <= highest_assigned_pid; i++) {
        if (Process &proc = processes[i]; proc.state == Process::STATE_WAITING) {
            if (proc.waiting_fid > 0 && art_async_done(proc.waiting_fid)) {
                proc.state = Process::STATE_READY;
                proc.context.eax = art_async_n_read(proc.waiting_fid);
                proc.waiting_fid = -1;
            }
        }
    }
}

size_t Scheduler::getCurrentProcessID() {
    return current_process_id;
}

EventQueue *Scheduler::getCurrentProcessEventQueue() {
    return processes[current_process_id].eventQueue;
}

uintptr_t Scheduler::getCurrentProcessPagingDirectory() {
    return processes[current_process_id].paging_table->get_phys_addr_of_page_dir();
}

void handle_expired_timers()
{
    if (!sleep_timers.head())
    {
        execution_counter = TSC_get_ticks();
        return;
    }
    const u64 ticks = TSC_get_ticks();
    long double elapsed_ms = (ticks - execution_counter) * 1000 / static_cast<long double>(cpuid_get_TSC_frequency());
    sleep_timers.iterate([elapsed_ms](sleep_timer_t* t) { t->counter -= elapsed_ms; });
    execution_counter = ticks;
    while (true) // until all counters less than 0 are accounted for
    {
        sleep_timer_t* timer = sleep_timers.find_if([](const sleep_timer_t& t) { return t.counter <= 0; });
        if (timer == nullptr) return;
        const size_t pid = timer->pid;
        sleep_timers.remove(timer);
        if (processes[pid].state == Process::STATE_SLEEPING)
        {
            processes[pid].state = Process::STATE_READY;
        }
    }
}

size_t get_oldest_process() {
    size_t ret_id = 0;
    u64 lowest = -1;
    for (size_t i = 2; i <= highest_assigned_pid; i++) {
        if (processes[i].state != Process::STATE_READY) continue;
        if (processes[i].last_executed < lowest) {
            ret_id = i;
            lowest = processes[i].last_executed;
        }
    }
    if (ret_id == 0) return 1;
    return ret_id;
}

size_t Scheduler::getNextProcessID() {
    const size_t next = get_oldest_process(); // dec and return split here for debugging purposes.
    return next;
}

void Scheduler::start_oneshot(u32 time_us) {
    if (lapic_timer->start_timer_us(time_us) < 0) {
#if ENABLE_SERIAL_LOGGING
        get_serial().log("Lapic failed to start timer?");
#endif
    };
}

//TODO: refactor. this no longer converts
void Scheduler::store_current_context(cpu_registers_t *r, const size_t PID) {
    art_string::memcpy(&processes[PID].context, r, sizeof(cpu_registers_t));
}

void Scheduler::set_current_context(cpu_registers_t *r, size_t PID) {
    art_string::memcpy(r, &processes[PID].context, sizeof(cpu_registers_t));
    asm volatile("mov %0, %%cr3" :: "r"(processes[PID].cr3_val) : "memory");
}

void Scheduler::sleep_ms(cpu_registers_t *r) {
    const size_t ms = r->ebx;
    sleep_timers.append(sleep_timer_t{current_process_id, ms});
    processes[current_process_id].state = Process::STATE_SLEEPING;
    execution_counter = TSC_get_ticks();
    processes[current_process_id].last_executed = execution_counter;
    schedule(r);
}

void Scheduler::mark_process_as_waiting(cpu_registers_t *r) {
    const int fd = r->ebx;
    processes[current_process_id].state = Process::STATE_WAITING;
    processes[current_process_id].waiting_fid = fd;
    schedule(r);
}

// When called from interrupt, the state is stored at this pointer loc, r.
void Scheduler::schedule(cpu_registers_t *const r) {
    clean_up_exited_threads();
    handle_expired_timers();
    check_finished_io_read();
    const size_t next_id = getNextProcessID();
    switch_process(r, next_id);
}


void LAPIC_handler(cpu_registers_t *const r) {
    Scheduler::schedule(r);
    // TODO: implement scheduler
}

// Exit is called by the program to tell the OS it is done.
void Scheduler::exit(cpu_registers_t *const r) {
    u32 status = r->ebx;
    LOG("Exiting ", processes[current_process_id].name, " PID: ", current_process_id, " with status: ", status);
    if (current_process_id == 2) {
        const int shell_file = art_open("b.art", 0);
        if (shell_file < 1) { LOG("CRITICAL: Failed to restart b.art"); }
        art_exec(shell_file);
        yield();
        art_close(shell_file);
    }
    processes[current_process_id].state = Process::STATE_EXITED;
    auto parent_id = processes[current_process_id].parent_pid;
    if (processes[parent_id].state == Process::STATE_PARKED) {
        processes[parent_id].state = Process::STATE_READY;
        processes[parent_id].context.eax = status;
    }
    size_t next = getNextProcessID();
    LOG("Post-exit process ID:", next);
    switch_process(r, next);
}

// Kill is supposed to send a command to the process to tell it to exit.
void Scheduler::kill(size_t target_pid) {
    processes[target_pid].state = Process::STATE_EXITED;
    auto parent_id = processes[current_process_id].parent_pid;
    if (processes[parent_id].state == Process::STATE_PARKED) {
        processes[parent_id].state = Process::STATE_READY;
    }
    // ???? what do here? I cannot switch because then it never returns?
    // switch_process(r, getNextProcessID());
    // kyield();
}

void Scheduler::create_idle_task() {
    const size_t next_process_id = getNextFreeProcessID();
    if (next_process_id + 1 >= max_processes) return; // TODO: Error
    auto *proc = &processes[next_process_id];
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
    proc->last_executed = -1;
    proc->priority = Process::PRIORITY_LOW;
    const auto nm = "idle_task";
    art_string::strncpy(proc->name, nm, MIN(32, art_string::strlen(nm)));
    proc->context = context;
    proc->eventQueue = new EventQueue();
    proc->cr3_val = kernel_pages().get_phys_addr_of_page_dir();
}

void Scheduler::execute_from_paging_table(PagingTableUser *PTU, const char *name_loc, const uintptr_t entry_point,
                                          const uintptr_t stack_vaddr, const uintptr_t stack_size) {
    auto name = name_loc;
    // Should copy stack contents but I am not sure if they are already ruined by this.R=
    size_t next_process_id = getNextFreeProcessID();
    const size_t parent_process_id = current_process_id;
    if (next_process_id + 1 >= max_processes) return; // TODO: Error

    auto *proc = &processes[next_process_id];
    *proc = Process{};

    cpu_registers_t context{};
    proc->user = true;
    proc->paging_table = PTU;
    uintptr_t stack_top;
    void *proc_stack;

    // TOOD: this needs to be replaced so that a malloc call can be done using flags instead.
    // TODO: the stack must be part of the user space memory map so this has to be remapped!
    if (stack_size == 0 or stack_vaddr == 0) {
        proc_stack = art_alloc(stack_size, stack_alignment);
        stack_top = reinterpret_cast<uintptr_t>(proc_stack) + stack_size;
    } else {
        proc_stack = reinterpret_cast<u8 *>(stack_vaddr);
        stack_top = stack_vaddr + stack_size;
    }

    LOG("Starting Process: ", name, " PID: ", next_process_id);
    context.esp = stack_top;
    context.cs = user_cs_offset | RPL_USER;
    context.ds = user_ds_offset | RPL_USER;
    context.es = user_ds_offset | RPL_USER;
    context.fs = user_ds_offset | RPL_USER;
    context.gs = user_ds_offset | RPL_USER;
    context.ss = user_ds_offset | RPL_USER;
    context.eip = reinterpret_cast<u32>(entry_point);
    context.eflags = default_eflags;

    // proc->eventQueue = new EventQueue();
    proc->cr3_val = proc->paging_table->get_phys_addr_of_page_dir();
    proc->last_executed = TSC_get_ticks();
    proc->start(parent_process_id, context, proc_stack, name, true);

    processes[parent_process_id].state = Process::STATE_PARKED;
}

PagingTableUser &Scheduler::getCurrentPagingTable() {
    return *processes[current_process_id].paging_table;
}
