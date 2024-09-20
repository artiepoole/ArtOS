//
// Created by artypoole on 18/07/24.
//

#ifndef PIT_H
#define PIT_H

#include "types.h"

void configure_pit(u32 hz);


void PIT_sleep_ms(u32 ms);


void pit_handler();

#endif //PIT_H
