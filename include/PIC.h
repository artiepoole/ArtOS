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
    void disable();
    void enable();
    void enableIRQ(u8 irq_id);
    void disableIRQ(u8 irq_id);
    void enableAll();
};



#endif //PIC_H
