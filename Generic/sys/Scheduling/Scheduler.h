//
// Created by artypoole on 21/11/24.
//

#ifndef SCHEDULER_H
#define SCHEDULER_H

#include  "types.h"
#include "Process.h"

size_t stack_size = 1024 * 1024; // 1MB stack default. Probably not enough.
constexpr size_t max_processes = 255;

#ifdef __cplusplus

class Scheduler
{
public:
    explicit Scheduler(void main_fn());

    int start();
    int fork();
    int exit();

    const Process* currentProcess();
    const Process* nextProcess();

private:
    u64 execution_counter = 0;
    size_t current_process_id = 0;
    size_t next_process_id = 0;
    size_t context_switch_period_ms = 100;
    Process processes[max_processes];
};

extern "C" {
#endif

int kstart(bool user);
int kfork();
int kexit();


#ifdef __cplusplus
}
#endif


#endif //PROCESS_H
