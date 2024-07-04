//
// Created by artypoole on 02/07/24.
//

#include "idt.h"

#include "serial.h"


void irq_remap()
{
    /* Normally, IRQs 0 to 7 are mapped to entries 8 to 15. This
    *  is a problem in protected mode, because IDT entry 8 is a
    *  Double Fault! Without remapping, every time IRQ0 fires,
    *  you get a Double Fault Exception, which is NOT actually
    *  what's happening. We send commands to the Programmable
    *  Interrupt Controller (PICs - also called the 8259's) in
    *  order to make IRQ0 to 15 be remapped to IDT entries 32 to
    *  47 */
    outb(0x20, 0x11);
    outb(0xA0, 0x11);
    outb(0x21, 0x20);
    outb(0xA1, 0x28);
    outb(0x21, 0x04);
    outb(0xA1, 0x02);
    outb(0x21, 0x01);
    outb(0xA1, 0x01);
    outb(0x21, 0x0);
    outb(0xA1, 0x0);
}

// __attribute__((noreturn))
void handle_div_by_zero()
{
    serial_write_string("Div by zero not handled. oops.\n");
}

void irq_0()
{
    serial_write_string("irq_0");
}


void register_to_serial(const registers* r)
{

    serial_write_string("int_no, err_code: ");
    serial_new_line();
    serial_write_hex(r->int_no, 4);
    serial_write_string(", ");
    serial_write_hex(r->err_code, 4);
    serial_new_line();

    serial_write_string("gs, fs, es, ds: ");
    serial_new_line();
    serial_write_hex(r->gs, 4);
    serial_write_string(", ");
    serial_write_hex(r->fs, 4);
    serial_write_string(", ");
    serial_write_hex(r->es, 4);
    serial_write_string(", ");
    serial_write_hex(r->ds, 4);
    serial_new_line();

    serial_write_string("edi, esi, ebp, esp, ebx, edx, ecx, eax;");
    serial_new_line();
    serial_write_hex(r->edi, 4);
    serial_write_string(", ");
    serial_write_hex(r->esi, 4);
    serial_write_string(", ");
    serial_write_hex(r->ebp, 4);
    serial_write_string(", ");
    serial_write_hex(r->esp, 4);
    serial_write_string(", ");
    serial_write_hex(r->ebx, 4);
    serial_write_string(", ");
    serial_write_hex(r->edx, 4);
    serial_write_string(", ");
    serial_write_hex(r->ecx, 4);
    serial_write_string(", ");
    serial_write_hex(r->eax, 4);
    serial_new_line();

    serial_write_string("eip, cs, eflags, useresp, ss;");
    serial_new_line();
    serial_write_hex(r->eip, 4);
    serial_write_string(", ");
    serial_write_hex(r->cs, 4);
    serial_write_string(", ");
    serial_write_hex(r->eflags, 4);
    serial_write_string(", ");
    serial_write_hex(r->useresp, 4);
    serial_write_string(", ");
    serial_write_hex(r->ss, 4);
    serial_new_line();


}

void exception_handler(const registers* r)
{
    register_to_serial(r);

    serial_write_string("Exception: ");
    serial_write_hex(r->int_no, 4);
    serial_new_line();

    if (r->int_no < 32)
    {
        /* Display the description for the Exception that occurred.
        *  In this tutorial, we will simply halt the system using an
        *  infinite loop */
        serial_write_string(exception_messages[r->int_no]);
        serial_new_line();
        switch (r->int_no)
        {
        case 0:
            handle_div_by_zero();
            break;
        default:
            serial_write_string("Unhandled exception. System Halted! ISR number: ");
            serial_write_int(r->int_no);
            serial_new_line();
            for (;;);
        }
    }
    else
    {
        serial_write_string("Unknown exception. System Halted!\n");
        for (;;);
    }
}

void irq_handler(const registers* r)
{
    serial_write_string("IRQ: ");
    const auto int_no = r->int_no;
    serial_write_int(int_no);
    serial_new_line();

    if (int_no>32)
    {
        switch (int_no-32)
        {
        case 0:
            irq_0();
        default:
            serial_write_string("Unhandled IRQ: ");
            serial_write_int(int_no);
            serial_new_line();
        }
    }

    /* If the IDT entry that was invoked was greater than 40
    *  (meaning IRQ8 - 15), then we need to send an EOI to
    *  the slave controller */
    if (int_no >= 40)
    {
        outb(0xA0, 0x20);
    }

    /* In either case, we need to send an EOI to the master
    *  interrupt controller too */
    outb(0x20, 0x20);
}



void idt_set_descriptor(const u8 vector, void* isr, const u8 flags)
{
    idt_entry_t* descriptor = &idt_entries[vector];

    descriptor->isr_low = reinterpret_cast<u32>(isr) & 0xFFFF;
    descriptor->kernel_cs = 0x10;
    // this value can be whatever offset your kernel code selector is in your GDT.
    // My entry point is 0x001005e0 so the offset is 0x0010(XXXX) (because of GRUB)
    descriptor->attributes = flags;
    descriptor->isr_high = reinterpret_cast<u32>(isr) >> 16;
    descriptor->reserved = 0;
}


void idt_install()
{
    /* also installs irq */
    serial_write_string("Remapping irq\n");
    irq_remap();

    idt_pointer.limit = (sizeof (idt_entry_t) * 48) - 1;
    idt_pointer.base = reinterpret_cast<uintptr_t>(&idt_entries[0]);

    for (u8 vector = 0; vector < 48; vector++)
    {
        idt_set_descriptor(vector, isr_stub_table[vector], 0x8E);
        idt_vectors[vector] = true;
    }

    serial_write_string("Setting IDT base and limit. ");
    serial_write_string("Base: ");
    serial_write_hex(idt_pointer.base, 4);
    serial_write_string(" Limit: ");
    serial_write_hex(idt_pointer.limit, 2);
    serial_new_line();
    __asm__ volatile ("lidt %0" : : "m"(idt_pointer)); // load the new IDT
    serial_write_string("LDT has been set\n");
    __asm__ volatile ("sti"); // set the interrupt flag
    serial_write_string("Interrupts enabled\n");
}




