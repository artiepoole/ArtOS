//
// Created by artypoole on 06/01/25.
//
#include "Process.h"

#include <cmp_int.h>
#include <string.h>

#include "EventQueue.h"

Process::Process()
{
    state = STATE_DEAD;
    parent_pid = -1;
    priority = PRIORITY_NORMAL;
    last_executed = 0;
    context = cpu_registers_t{};
    stack = NULL;
    name[0] = '\0';
    eventQueue = NULL;
}

void Process::reset()
{
    state = STATE_DEAD;
    parent_pid = -1;
    priority = PRIORITY_NORMAL;
    last_executed = 0;
    context = cpu_registers_t{};
    stack = NULL;
    name[0] = '\0';
    delete eventQueue;
    eventQueue = NULL;
}


void Process::start(const size_t parent_id, const cpu_registers_t& new_context, void* new_stack, const char* new_name)
{
    parent_pid = parent_id;
    state = STATE_READY;
    priority = PRIORITY_NORMAL;
    last_executed = 0;
    context = new_context;
    stack = new_stack;
    eventQueue = new EventQueue();
    strncpy(name, new_name, MIN(32, strlen(name)));
}

Process::~Process()
{
    if (eventQueue != NULL)
    {
        delete eventQueue;
    }
}
