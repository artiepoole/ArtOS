//
// Created by artypoole on 02/07/24.
//

#ifndef IDT_H
#define IDT_H
#include "CPU.h"
#include "PIC.h"


#define IDT_STUB_COUNT 49
#define IDT_SPURIOUS_ID 0xFF


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
