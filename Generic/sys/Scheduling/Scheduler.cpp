//
// Created by artypoole on 02/01/25.
//
#include "Scheduler.h"

#include <GDT.h>
#include <LocalAPIC.h>
#include <logging.h>
#include <stdlib.h>
#include <doomgeneric/doomkeys.h>

#include "memory.h"


// typedef struct {
//     uint32_t cs;
//     uint32_t eip;
// } eip_t;

size_t stack_alignment = 16;

u32 default_eflags = 0x206;
Scheduler* scheduler_instance = nullptr;
u64 execution_counter = 0;
size_t current_process_id = 0;
size_t next_process_id = 1;
size_t context_switch_period_ms = 100;
Process processes[max_processes];
LocalAPIC* lapic_timer = nullptr;


Scheduler::Scheduler(void (*main_func)(), LocalAPIC* timer)
{
    // Store current state in process[0]
    // Set up first Lapic one shot
    scheduler_instance = this;
    lapic_timer = timer;
    processes[0].state = Process::STATE_PARKED;
    execf(main_func);
}

Scheduler::~Scheduler()
{
    scheduler_instance = nullptr;
}

Scheduler& Scheduler::get()
{
    return *scheduler_instance;
}


void Scheduler::switch_process(const size_t new_PID)
{
    const auto old_pid = current_process_id;
    current_process_id = new_PID;
    store_current_context(old_pid);
    const auto priority = processes[current_process_id].priority;
    start_oneshot(context_switch_period_ms * priority);
    execution_counter += priority;
    set_current_context(new_PID);
}

void Scheduler::switch_process(const cpu_registers_t* r, const size_t new_PID)
{
    convert_current_context(r, current_process_id);
    current_process_id = new_PID;
    const auto priority = processes[current_process_id].priority;
    start_oneshot(context_switch_period_ms * priority);
    execution_counter += priority;
    set_current_context(current_process_id);
}

// size_t Scheduler::getCurrentProcessID()
// {
// }

void Scheduler::exit(const size_t pid)
{
    aligned_free(processes[pid].stack);
    processes[pid].reset();
}

size_t Scheduler::getNextProcessID()
{
    size_t target_pid = 0;
    size_t lowest = -1;
    for (size_t i = 0; i < max_processes; i++)
    {
        if (processes[i].state != Process::STATE_READY) continue;
        if (processes[i].last_executed < lowest)
        {
            target_pid = i;
            lowest = processes[i].last_executed;
        }
    }
    return target_pid;
}

void Scheduler::start_oneshot(u32 time_ms)
{
    lapic_timer->start_timer(time_ms);
}

void Scheduler::store_current_context(size_t PID)
{
    auto context = processes[PID].context; // copy of context
    __asm__ volatile (
        "movl %%eax, %0\n\t" // Save eax
        "movl %%ebx, %1\n\t" // Save ebx
        "movl %%ecx, %2\n\t" // Save ecx
        "movl %%edx, %3\n\t" // Save edx
        "movl %%esi, %4\n\t" // Save esi
        "movl %%edi, %5\n\t" // Save edi
        "movl %%ebp, %6\n\t" // Save ebp
        "movl %%esp, %7\n\t" // Save esp
        "pushfl\n\t" // Push EFLAGS to stack
        "popl %8\n\t" // Save EFLAGS
        "movw %%cs, %9\n\t" // Save CS
        "movw %%ds, %10\n\t" // Save DS
        "movw %%es, %11\n\t" // Save ES
        "movw %%fs, %12\n\t" // Save FS
        "movw %%gs, %13\n\t" // Save GS
        "movw %%ss, %14\n\t" // Save SS
        "call 1f\n\t" // Get EIP by calling a label
        "1: popl %15\n\t" // Save EIP
        : "=m"(context.eax), "=m"(context.ebx), "=m"(context.ecx), "=m"(context.edx),
        "=m"(context.esi), "=m"(context.edi), "=m"(context.ebp), "=m"(context.esp),
        "=m"(context.eflags), "=m"(context.cs), "=m"(context.ds), "=m"(context.es),
        "=m"(context.fs), "=m"(context.gs), "=m"(context.ss), "=m"(context.eip)
        :
        : "memory"
    );
    processes[PID].context = context;
}

void Scheduler::convert_current_context(const cpu_registers_t* r, const size_t PID)
{
    const auto context = &processes[PID].context;
    context->esp = r->esp;
    context->cs = r->cs;
    context->ds = r->ds;
    context->ss = r->ss;
    context->eip = r->eip;
    context->eflags = r->eflags;
    context->ecx = r->ecx;
    context->edx = r->edx;
    context->eax = r->eax;
    context->ebx = r->ebx;
    context->esi = r->esi;
    context->edi = r->edi;
    context->ebp = r->ebp;
    context->es = r->es;
    context->fs = r->fs;
    context->gs = r->gs;
}

void Scheduler::set_current_context(size_t PID)
{
    const auto context = processes[PID].context;
    // const eip_t ptr = {context.cs, context.eip};
    __asm__ volatile (
        "movw %0, %%ds\n\t" // Restore DS
        "movw %1, %%ss\n\t" // Restore SS
        "movl %2, %%edi\n\t" // Restore edi
        "movl %3, %%esi\n\t" // Restore esi
        "movl %4, %%ebp\n\t" // Restore ebp
        // TODO: if I choose to restore more values, they will go here.
        "movl %6, %%esp\n\t" // Restore esp i.e. the stack
        "push %7\n\t" // Push CS (segment) onto the stack
        "push %8\n\t" // Push EIP (offset) onto the stack
        "ljmp *(%%esp)\n\t"
        :
        : "m"(context.ds), "m"(context.ss), "m"(context.edi), "m"(context.esi), "m"(context.ebp),
        "m"(context.eflags), "r"(context.esp), "r"(context.cs), "r"(context.eip)
        : "memory"
    );
    // no return.
}

// When called without stored registers.
void Scheduler::schedule()
{
    switch_process(getNextProcessID());
}

// When called from interrupt, the state is stored at this pointer loc, r.
void Scheduler::schedule(const cpu_registers_t* r)
{
    const size_t next_id = getNextProcessID();
    if (next_id == current_process_id)
    {
        start_oneshot(context_switch_period_ms * processes[next_id].priority);
        return;
    }
    switch_process(r, next_id);
}

// Create new process
void Scheduler::execf(void (*func)()) // if user mode then the CS and DS should be different
{
    if (next_process_id + 1 >= max_processes) return; // TODO: Error
    void* proc_stack = aligned_malloc(stack_size, stack_alignment);
    void* stack_top = static_cast<u8*>(proc_stack) + stack_size;
    cpu_context_t context{};
    context.esp = reinterpret_cast<u32>(stack_top);
    context.cs = kernel_cs_offset;
    context.ds = kernel_ds_offset;
    context.ss = kernel_ds_offset;
    context.eip = reinterpret_cast<u32>(func);
    context.eflags = default_eflags;
    size_t pid = next_process_id;
    next_process_id++;

    auto* proc = &processes[pid];
    proc->pid = next_process_id++;
    proc->state = Process::STATE_READY;
    proc->priority = Process::PRIORITY_NORMAL;
    proc->last_executed = execution_counter;
    proc->context = context;
    proc->stack = stack_top;

    switch_process(pid);
}


void LAPIC_handler(const cpu_registers_t* r)
{
    Scheduler::schedule(r);
    // TODO: implement scheduler
}
