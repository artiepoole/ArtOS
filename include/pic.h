//
// Created by artypoole on 04/07/24.
//

#ifndef PIC_H
#define PIC_H
#include "types.h"
#include "serial.h"
#include "ports.h"



class PIC
{
public:
    PIC();
    void disable();
    void enable();
    void enableIRQ(u8 i);
    void enableAll();
};

void timerHandler();
void sleep(u32 ms);
// extern "C"
void configurePit(u32 hz);


#endif //PIC_H
