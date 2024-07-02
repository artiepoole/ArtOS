//
// Created by artypoole on 02/07/24.
//

#ifndef IDT_H
#define IDT_H
#include "types.h"
#include <stdint.h>

static bool vectors[32];

extern void* isr_stub_table[];

typedef struct {
    u16    isr_low;      // The lower 16 bits of the ISR's address
    u16    kernel_cs;    // The GDT segment selector that the CPU will load into CS before calling the ISR
    u8     reserved;     // Set to zero
    u8     attributes;   // Type and attributes; see the IDT page
    u16    isr_high;     // The higher 16 bits of the ISR's address
} __attribute__((packed)) idt_entry_t;

typedef struct {
    u16	limit;
    u32	base;
} __attribute__((packed)) idtr_t;

static idtr_t idtr;

__attribute__((aligned(0x10)))
static idt_entry_t idt[256]; // Create an array of IDT entries; aligned for performance

void idt_set_descriptor(u8 vector, void* isr, u8 flags);
extern "C"
void exception_handler(void);
void idt_init(void);


#endif //IDT_H
