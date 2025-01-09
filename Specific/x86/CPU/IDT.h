//
// Created by artypoole on 02/07/24.
//

#ifndef IDT_H
#define IDT_H
#include "CPU.h"
#include "PIC.h"


#define IDT_STUB_COUNT 52
#define IDT_SPURIOUS_ID 240
#define SYSCALL_ID 50
inline const u8 syscall_id = SYSCALL_ID;

enum
{
    PIC_IRQ,
    KEYBOARD_IRQ,
    CASCADE_IRQ,
    COM2_IRQ,
    COM1_IRQ,
    LPT2_IRQ,
    FLOPPY_IRQ,
    LPT1_IRQ,
    RTC_IRQ,
    IRQ_9,
    IRQ_10,
    IRQ_11,
    PS2_MOUSE_IRQ,
    FPU_IRQ,
    IDE_PRIMARY_IRQ,
    IDE_SECONDARY_IRQ,
    LAPIC_IRQ,
    LAPIC_CALIBRATE_IRQ,
    SYSCALL_IRQ = SYSCALL_ID - 32,
    SPURIOUS_IRQ = IDT_SPURIOUS_ID - 32
};

class IDT
{
public:
    IDT();

private:
    static void _setDescriptor(u8 idt_index, u8 flags);
};

extern "C"
void exception_handler(cpu_registers_t* const r);

extern "C"
void irq_handler(cpu_registers_t* const r);

extern volatile bool ATA_transfer_in_progress;
extern volatile bool BM_transfer_in_progress;

#endif //IDT_H
