//
// Created by artypoole on 04/07/24.
//

#ifndef PIC_H
#define PIC_H
#include "types.h"
#include "ports.h"
#include "system.h"

class PIC
{
public:
    PIC();
    void disable();
    void irq_remap();
    void enable();
    void enable_irq0();
    void enable_all();
};

void timer_handler();
void sleep(u32 ms);
// extern "C"
void configure_pit(u32 hz);


#endif //PIC_H
