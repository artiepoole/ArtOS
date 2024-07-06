//
// Created by artypoole on 04/07/24.
//

#ifndef PIC_H
#define PIC_H
#include "types.h"
#include "ports.h"
#include "serial.h"


void pic_disable();
void pic_irq_remap();
void pic_enable_irq0();
void pic_enable_all();
void timer_handler();
void sleep(u32 ms);
// extern "C"
void configure_pit(u32 hz);


#endif //PIC_H
