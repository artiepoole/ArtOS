// ArtOS - hobby operating system by Artie Poole
// Copyright (C) 2025 Stuart Forbes Poole <artiepoole>
//
//     This program is free software: you can redistribute it and/or modify
//     it under the terms of the GNU General Public License as published by
//     the Free Software Foundation, either version 3 of the License, or
//     (at your option) any later version.
//
//     This program is distributed in the hope that it will be useful,
//     but WITHOUT ANY WARRANTY; without even the implied warranty of
//     MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//     GNU General Public License for more details.
//
//     You should have received a copy of the GNU General Public License
//     along with this program.  If not, see <https://www.gnu.org/licenses/>

//
// Created by artypoole on 02/07/24.
//

#ifndef IDT_H
#define IDT_H
#include "CPU.h"
#include "PIC.h"


#define IDT_STUB_COUNT 51
#define IDT_SPURIOUS_ID 0xFF


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
    SPURIOUS_IRQ = 208
};

class IDT
{
public:
    IDT();
private:
    static void _setDescriptor(u8 idt_index, void* isr_stub, u8 flags);
};

extern "C"
void exception_handler(cpu_registers_t* const r);

extern "C"
void irq_handler(cpu_registers_t* const r);

extern volatile bool ATA_transfer_in_progress;
extern volatile bool BM_transfer_in_progress;

#endif //IDT_H
