//
// Created by artypoole on 18/07/24.
//

#ifndef PIT_H
#define PIT_H

#include "types.h"

void configurePit(u32 hz);


void sleep(u32 ms);


void pit_handler();

#endif //PIT_H
