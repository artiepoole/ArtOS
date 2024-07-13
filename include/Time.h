//
// Created by artypoole on 13/07/24.
//

#ifndef TIME_H
#define TIME_H



#include "Serial.h"

void configurePit(u32 hz);

void sleep(u32 ms);

void timerHandler();

#endif //TIME_H
