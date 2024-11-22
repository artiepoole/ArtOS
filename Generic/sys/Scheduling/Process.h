//
// Created by artypoole on 21/11/24.
//

#ifndef PROCESS_H
#define PROCESS_H

#include "types.h"
#include "CPU.h"

#ifdef __cplusplus
//https://en.wikipedia.org/wiki/Task_state_segment
class Process
{
public:
    Process();
    ~Process() = default;

    enum State_t
    {
        STATE_READY,
        STATE_PARKED,
        STATE_DEAD,
    };

    bool isParked() { return state == STATE_PARKED; }
    bool isDead() { return state == STATE_DEAD; }

private:
    u32 pid;
    u32 state;
    char priority;
    size_t last_executed;
    cpu_context_t context;
    //    u32 base_vaddr;
    // char name[32]; this can be stored in an equivalent of proc?
    char* stack[];
};

extern "C" {
#endif


#ifdef __cplusplus
}
#endif

#endif //PROCESS_H
