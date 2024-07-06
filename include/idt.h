//
// Created by artypoole on 02/07/24.
//

#ifndef IDT_H
#define IDT_H
#include "system.h"
#include "pic.h"


typedef struct
{
    u16 isr_low; // The lower 16 bits of the ISR's address
    u16 kernel_cs; // The GDT segment selector that the CPU will load into CS before calling the ISR
    u8 reserved; // Set to zero
    u8 attributes; // Type and attributes; see the IDT page
    u16 isr_high; // The higher 16 bits of the ISR's address
} __attribute__((packed)) idt_entry_t;

typedef struct
{
    u16 limit;
    u32 base;
} __attribute__((packed)) idt_ptr_t;


static bool idt_vectors[48];

extern void* isr_stub_table[];


static idt_ptr_t idt_pointer;

__attribute__((aligned(0x10)))
static idt_entry_t idt_entries[256]; // Create an array of IDT entries; aligned for performance

void idt_set_descriptor(u8 idt_table_index, void* isr_stub, u8 flags);

extern "C"
void exception_handler(const registers* r);
void idt_install(void);

extern "C"
void irq_handler(const registers* r);
void irq_install(void);



inline char exception_messages[][40] =
{
    "div by zero",
    "debug exception",
    "non-maskable interrupt exception",
    "breakpoint exception",
    "into detected overflow exception",
    "out of bounds exception",
    "invalid opcode exception",
    "double fault exception",
    "coprocessor segment overrun exception",
    "bad TSS exception",
    "segment not present exception",
    "stack fault exception",
    "general protection fault exception",
    "page fault exception",
    "unknown interrupt exception",
    "coprocessor fault exception",
    "alignment check exception",
    "machine check exception",
    "reserved exceptions",
    "reserved exceptions",
    "reserved exceptions",
    "reserved exceptions",
    "reserved exceptions",
    "reserved exceptions",
    "reserved exceptions",
    "reserved exceptions",
    "reserved exceptions",
    "reserved exceptions",
    "reserved exceptions",
    "reserved exceptions",
    "reserved exceptions"
};

#endif //IDT_H
