//
// Created by artypoole on 21/11/24.
//

#ifndef SCHEDULER_H
#define SCHEDULER_H

#include <LocalAPIC.h>

#include  "types.h"
#include "Process.h"

inline size_t stack_size = 1024 * 1024; // 1MB stack default. Probably not enough.
constexpr size_t max_processes = 255;

#ifdef __cplusplus

class Scheduler
{
public:
    Scheduler(void (*main_func)(), LocalAPIC* timer);
    ~Scheduler();
    static Scheduler& get();
    // void start(size_t PID);
    static void switch_process(size_t new_PID);
    void switch_process(const cpu_registers_t* r, size_t new_PID);

    // Only takes void foo() types atm. No support for input variables
    static void execf(void (*func)());

    static void fork();
    static void exit(size_t pid);

    // size_t getCurrentProcessID();
    static size_t getNextProcessID();

    static void start_oneshot(u32 time_ms);
    static void store_current_context(size_t PID);
    static void convert_current_context(const cpu_registers_t* r, size_t PID);
    static void set_current_context(size_t PID);

    static void schedule(const cpu_registers_t* r);
    static void schedule();
};


extern "C" {
#endif

// int kexecf(void (*main_func)(), bool user);
int kfork();
int kyield();
int kexit(size_t pid);


#ifdef __cplusplus
}
#endif


#endif //PROCESS_H
