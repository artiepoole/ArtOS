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


class Scheduler
{
public:
    Scheduler(void (*main_func)(), char* name, LocalAPIC* timer, EventQueue* kernel_queue);
    ~Scheduler();
    static Scheduler& get();
    // void start(size_t PID);
    // static void switch_process(size_t new_PID);
    static void switch_process(cpu_registers_t* r, size_t new_PID);

    static size_t getNextFreeProcessID();
    static size_t getMaxAliveProcessID();
    // Only takes void foo() types atm. No support for input variables
    static void execf(void (*func)(), const char* name, bool user = false);

    // static void fork();
    static void exit(int status);
    static void kill(cpu_registers_t* r);
    static void create_idle_task();

    static void clean_up_exited_threads();
    static size_t getCurrentProcessID();
    static EventQueue* getCurrentProcessEventQueue();
    static bool isCurrentProcessUser();
    static bool isProcessUser(size_t PID);
    static size_t getNextProcessID();

    static void start_oneshot(u32 time_ms);
    // static void store_current_context(size_t PID);
    static void convert_current_context(cpu_registers_t* r, size_t PID);
    static void set_current_context(cpu_registers_t* r, size_t PID);

    static void schedule(cpu_registers_t* r);
    // static void schedule();

    static void sleep_ms(u32 ms);
};


#endif //PROCESS_H
