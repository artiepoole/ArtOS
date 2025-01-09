//
// Created by artypoole on 06/07/24.
//

#ifndef KEYBOARD_H
#define KEYBOARD_H

#include "types.h"
#include "event.h"


extern const char key_map[128];
extern const char shift_map[128];


void keyboardHandler();

class EventQueue
{
public:
    EventQueue();
    ~EventQueue();
    explicit EventQueue(size_t max_items);
    void addEvent(const event_t& event);
    [[nodiscard]] bool pendingEvents() const;
    event_t getEvent();

private:
    size_t max_len = 0;
    event_t* _event_queue;
    size_t _unread_counter;
    size_t _write_index;
    size_t _read_index;
};


#endif //KEYBOARD_H
