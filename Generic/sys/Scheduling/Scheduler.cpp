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
    auto old_pid = current_process_id;
    current_process_id = new_PID;
    store_current_context(old_pid);
    start_oneshot();
    set_current_context(new_PID);
}

void Scheduler::start_oneshot()
{
    lapic_timer->start_timer(context_switch_period_ms);
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
        "movl %6, %%esp\n\t" // Restore esp // stack
        "push %7\n\t"      // Push CS (segment) onto the stack
        "push %8\n\t"      // Push EIP (offset) onto the stack
        "ljmp *(%%esp)\n\t"
        :
        : "m"(context.ds), "m"(context.ss), "m"(context.edi), "m"(context.esi), "m"(context.ebp),
        "m"(context.eflags), "r"(context.esp), "r"(context.cs), "r"(context.eip)
        : "memory"
    );

//     __asm__ volatile (
//     "movw %0, %%cs\n\t"    // Restore CS
//     :
//     : "m"(context.cs)
//     );
//
//     __asm__ volatile (
//         "movw %0, %%ds\n\t"    // Restore DS
//         :
//         : "m"(context.ds)
//     );
//
//     __asm__ volatile (
//         "movw %0, %%ss\n\t"    // Restore SS
//         :
//         : "m"(context.ss)
//     );
//
//
//
//     __asm__ volatile (
//         "movl %0, %%edi\n\t"   // Restore edi
//         :
//         : "m"(context.edi)
//     );
//
//     __asm__ volatile (
//         "movl %0, %%esi\n\t"   // Restore esi
//         :
//         : "m"(context.esi)
//     );
//
//     __asm__ volatile (
//         "movl %0, %%edx\n\t"   // Restore edx
//         :
//         : "m"(context.edx)
//     );
//
//     __asm__ volatile (
//         "movl %0, %%ebp\n\t"   // Restore ebp
//         :
//         : "m"(context.ebp)
//     );
//
//     // __asm__ volatile (
//     //
//     //     "ljmp %0, %1\n\t"    // Perform a far jump, loading new CS and EIP
//     //     :
//     //     : "m"(context.cs), "m"(context.eip)  // context.cs contains the new CS, context.eip contains the new EIP
//     // );
//
//     __asm__ volatile (
//     "movl %0, %%esp\n\t"   // Restore esp (stack pointer)
//     :
//     : "m"(context.esp)
// );
//     __asm__ volatile (
//        // Load the new CS into the CS register
//     "ljmp %0\n"           // Jump to the new EIP (the target address)
//     :
//     : "m"(ptr)  // context.cs and context.eip hold the new CS and EIP
//     : "memory"
// );
    // no return.
}

void Scheduler::schedule()
{
    LOG("Scheduling.");
    asm("hlt");
    // TODO: store current context, get next process.
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
    proc->last_executed = 0;
    proc->context = context;
    proc->stack = stack_top;

    switch_process(pid);
}


void LAPIC_handler(const cpu_registers_t* r)
{
    LOG("LAPIC INTERRUPT. r pointer: ", reinterpret_cast<u32>(r));
    Scheduler::schedule();
    // TODO: implement scheduler
}
