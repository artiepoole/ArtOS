//
// Created by artypoole on 02/07/24.
//

#include "idt.h"

#include "serial.h"


__attribute__((noreturn))

void exception_handler() {
    serial_write_string("Interrupted.");
}


void idt_set_descriptor(const u8 vector, void* isr, const u8 flags) {
    idt_entry_t* descriptor = &idt[vector];

    descriptor->isr_low        = reinterpret_cast<u32>(isr) & 0xFFFF;
    descriptor->kernel_cs      = 0x10; // this value can be whatever offset your kernel code selector is in your GDT 0x001005e0
    descriptor->attributes     = flags;
    descriptor->isr_high       = reinterpret_cast<u32>(isr) >> 16;
    descriptor->reserved       = 0;
}


void idt_init() {
    serial_write_string("Beginning idt init sequence\n");
    idtr.base = reinterpret_cast<uintptr_t>(&idt[0]);
    idtr.limit = static_cast<u16>(sizeof(idt_entry_t)) * 32 - 1;
    serial_write_string("Setting idt descriptors\n");
    for (u8 vector = 0; vector < 32; vector++) {
        idt_set_descriptor(vector, isr_stub_table[vector], 0x8E);
        vectors[vector] = true;
    }
    serial_write_string("doing asm bit\n");
    __asm__ volatile ("lidt %0" : : "m"(idtr)); // load the new IDT
    serial_write_string("doing asm bit 2\n");
    __asm__ volatile ("sti"); // set the interrupt flag
    serial_write_string("asm done\n");
}
