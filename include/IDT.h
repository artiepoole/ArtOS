//
// Created by artypoole on 02/07/24.
//

#ifndef IDT_H
#define IDT_H
#include "system.h"
#include "PIC.h"
#include "EventQueue.h"
#include "Time.h"




class IDT
{
public:
    IDT();
private:
    static void _setDescriptor(u8 idt_index, void* isr_stub, u8 flags);
};

extern "C"
void exception_handler(const registers* r);

extern "C"
void irq_handler(const registers* r);



#endif //IDT_H
