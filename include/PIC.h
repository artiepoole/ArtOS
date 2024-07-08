//
// Created by artypoole on 04/07/24.
//

#ifndef PIC_H
#define PIC_H
#include "types.h"
#include "Serial.h"
#include "ports.h"



class PIC
{
public:
    PIC();
    static void disable();
    static void enable();
    static void enableIRQ(u8 i);
    static void enableAll();
};

void timerHandler();
void sleep(u32 ms);
// extern "C"
void configurePit(u32 hz);


#endif //PIC_H
