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
    user = false;
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
    if (eventQueue != NULL)
    {
        delete eventQueue;
        eventQueue = NULL;
    }
    user = false;
}


void Process::start(const size_t parent_id, const cpu_registers_t& new_context, void* new_stack, const char* new_name, const bool is_user)
{
    parent_pid = parent_id;
    state = STATE_READY;
    priority = PRIORITY_NORMAL;
    last_executed = 0;
    context = new_context;
    if (is_user)
    {
        stack = new_stack;
    }
    else
    {
        stack = NULL;
    }
    eventQueue = new EventQueue();
    strncpy(name, new_name, MIN(32, strlen(name)));
    user = is_user;
}

Process::~Process()
{
    if (eventQueue != NULL)
    {
        delete eventQueue;
    }
}
