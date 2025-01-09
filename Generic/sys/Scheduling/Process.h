//
// Created by artypoole on 21/11/24.
//

#ifndef PROCESS_H
#define PROCESS_H

#include "types.h"
#include "CPU.h"

class EventQueue;

#ifdef __cplusplus
//https://en.wikipedia.org/wiki/Task_state_segment
struct Process
{
    Process();
    void reset();
    void start(size_t parent_id, const cpu_registers_t& new_context, void* new_stack, const char* new_name, bool is_user);
    ~Process();

    enum State_t
    {
        STATE_DEAD,
        STATE_EXITED,
        STATE_SLEEPING,
        STATE_PARKED,
        STATE_READY,
    };

    // Used to scale execution duration.
    enum Priority_t
    {
        PRIORITY_LOW = 1,
        PRIORITY_NORMAL = 2,
        PRIORITY_HIGH = 4,
    };

    bool isParked() { return state == STATE_PARKED; }
    bool isDead() { return state == STATE_DEAD; }

    u32 parent_pid;
    State_t state;
    Priority_t priority;
    size_t last_executed;
    cpu_registers_t context;
    //    u32 base_vaddr;
    char name[32]; //this can be stored in an equivalent of proc?
    void* stack;
    EventQueue* eventQueue;
    bool user;
};


#endif

#endif //PROCESS_H
