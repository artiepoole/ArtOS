//
// Created by artypoole on 02/07/24.
//

#ifndef IDT_H
#define IDT_H
#include "system.h"
#include "PIC.h"
#include "EventQueue.h"
#include "Timers.h"




class IDT
{
public:
    IDT();
private:
    static void _setDescriptor(u8 idt_index, void* isr_stub, u8 flags);
};

extern "C"
void exception_handler(const cpu_registers_t* r);

extern "C"
void irq_handler(const cpu_registers_t* r);



#endif //IDT_H
