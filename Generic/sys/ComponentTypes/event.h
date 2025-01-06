//
// Created by artypoole on 06/01/25.
//

#ifndef EVENT_H
#define EVENT_H

#include "types.h"

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

#endif //EVENT_H
