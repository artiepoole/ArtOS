//
// Created by artypoole on 06/07/24.
//

#ifndef KEYBOARD_H
#define KEYBOARD_H


#include "ports.h"
#include "serial.h"
#include "vga.h"

extern char key_map[128];

enum EVENT_TYPE
{
    NULL_EVENT = 0,
    TICK = 1,
    DRAW_CALL = 2,
    KEY_DOWN = 3,
    KEY_UP = 4,
    MOUSE = 5,

};

struct event_data_t
{
    u32 lower_data;
    u32 upper_data;
};

struct event_t
{
    EVENT_TYPE type;
    event_data_t data;
};


void keyboard_handler();

class EventQueue
{
public:
    EventQueue();
    ~EventQueue();
    static EventQueue& getInstance();
    void addEvent(const event_t& event);
    bool pendingEvents() const;
    event_t getEvent();
private:
    static constexpr size_t max_len = 1024;
    event_t event_queue[max_len];
    size_t _unread_counter;
    size_t _write_index;
    size_t _read_index;

};


#endif //KEYBOARD_H
