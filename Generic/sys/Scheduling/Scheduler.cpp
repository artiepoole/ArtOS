//
// Created by artypoole on 02/01/25.
//
#include "Scheduler.h"

#include <cmp_int.h>
#include <GDT.h>
#include <LocalAPIC.h>
#include <logging.h>
#include <SMBIOS.h>
#include <stdlib.h>
#include <TSC.h>
#include <doomgeneric/doomkeys.h>

#include "CPUID.h"
#include "kernel.h"
#include "memory.h"
#include "LinkedList.h"
#include "PIT.h"


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
size_t next_process_id = 1;
size_t context_switch_period_ms = 100;
Process processes[max_processes];
LocalAPIC* lapic_timer = nullptr;


LinkedList<sleep_timer_t> sleep_timers;


Scheduler::Scheduler(void (*main_func)(), LocalAPIC* timer)
{
    // Store current state in process[0]
    // Set up first Lapic one shot
    scheduler_instance = this;
    lapic_timer = timer;
    processes[0].state = Process::STATE_PARKED;
    execution_counter = TSC_get_ticks();
    execf(main_func, "scheduler");
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
    if (new_PID == 0)
    {
        start_oneshot(context_switch_period_ms);
        // TODO: check for waiting timers and then wait for timers, or execute process 1.
        return;
        // while (true);
    }
    const auto old_pid = current_process_id;
    current_process_id = new_PID;
    if (processes[old_pid].state != Process::STATE_DEAD) store_current_context(old_pid);
    const auto priority = processes[current_process_id].priority;
    start_oneshot(context_switch_period_ms * priority);
    execution_counter = TSC_get_ticks();
    set_current_context(new_PID);
}


void Scheduler::switch_process(const cpu_registers_t* r, const size_t new_PID)
{
    if (new_PID == 0)
    {
        start_oneshot(context_switch_period_ms);
        while (true);
    }
    convert_current_context(r, current_process_id);
    current_process_id = new_PID;
    const auto priority = processes[current_process_id].priority;
    start_oneshot(context_switch_period_ms * priority);
    execution_counter = TSC_get_ticks();
    set_current_context(current_process_id);
}

// size_t Scheduler::getCurrentProcessID()
// {
// }

void Scheduler::exit(int status)
{
    size_t pid = current_process_id;
    aligned_free(processes[pid].stack);

    LOG("Exiting ", processes[pid].name, " PID: ", pid, " with status: ", status);
    processes[pid].reset();
    processes[1].state = Process::STATE_READY;
    schedule();
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
        if (timer == nullptr) break;
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
    for (size_t i = 0; i < max_processes; i++)
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
    handle_expired_timers();

    return get_oldest_process();
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

void Scheduler::sleep_ms(const u32 ms)
{
    sleep_timers.append(sleep_timer_t{current_process_id, ms});
    processes[current_process_id].state = Process::STATE_PARKED;
    // TODO: it might be good to start a oneshot of this duration if the sleep time is smaller than one period.
    // i.e. start_oneshot(MIN(ms, context_switch_period_ms));
    schedule();
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
void Scheduler::execf(void (*func)(), char* name) // if user mode then the CS and DS should be different
{
    processes[current_process_id].state = Process::STATE_PARKED;
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
    proc->last_executed = 0;
    proc->context = context;
    proc->stack = proc_stack;
    // TODO: safe max len
    strncpy(proc->name, name, MIN(32, strlen(name)));
    switch_process(pid);
}


void LAPIC_handler(const cpu_registers_t* r)
{
    Scheduler::schedule(r);
    // TODO: implement scheduler
}
