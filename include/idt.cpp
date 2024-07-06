//
// Created by artypoole on 02/07/24.
//

#include "idt.h"

bool ignore_4 = false;

u16 KERNEL_CS = 0x0010;
u16 KERNEL_DS = 0x0018;
void register_to_serial(const registers* r)
{
    serial_write_string("int_no, err_code: ");
    serial_new_line();
    serial_write_hex(r->int_no);
    serial_write_string(", ");
    serial_write_hex(r->err_code);
    serial_new_line();

    serial_write_string("gs, fs, es, ds: ");
    serial_new_line();
    serial_write_hex(r->gs);
    serial_write_string(", ");
    serial_write_hex(r->fs);
    serial_write_string(", ");
    serial_write_hex(r->es);
    serial_write_string(", ");
    serial_write_hex(r->ds);
    serial_new_line();

    serial_write_string("edi, esi, ebp, esp, ebx, edx, ecx, eax;");
    serial_new_line();
    serial_write_hex(r->edi);
    serial_write_string(", ");
    serial_write_hex(r->esi);
    serial_write_string(", ");
    serial_write_hex(r->ebp);
    serial_write_string(", ");
    serial_write_hex(r->esp);
    serial_write_string(", ");
    serial_write_hex(r->ebx);
    serial_write_string(", ");
    serial_write_hex(r->edx);
    // serial_write_string(", ");
    // serial_write_hex(r->ecx, 4);
    // serial_write_string(", ");
    // serial_write_hex(r->eax, 4);
    serial_new_line();

    serial_write_string("eip, cs, eflags, useresp, ss;");
    serial_new_line();
    serial_write_hex(r->eip);
    serial_write_string(", ");
    serial_write_hex(r->cs);
    serial_write_string(", ");
    serial_write_hex(r->eflags);
    serial_write_string(", ");
    serial_write_hex(r->useresp);
    serial_write_string(", ");
    serial_write_hex(r->ss);
    serial_new_line();
}

// __attribute__((noreturn))
void handle_div_by_zero(const registers* r)
{
    serial_write_string("Div by zero not handled. oops.\n");
    register_to_serial(r);
}

// void irq_0()
// {
//     timer_handler();
// }



extern "C"
void exception_handler(const registers* r)
{
    register_to_serial(r);

    serial_write_string("Exception: ");
    // serial_write_hex(r->int_no, 4);
    // serial_new_line();

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
            // handle_div_by_zero(r);
            return;
        default:
            serial_write_string("Unhandled exception. System Halted!");
            for (;;);
        }
    }
    else
    {
        serial_write_string("Unknown exception. System Halted!\n");
        for (;;);
    }
}
extern "C"
void irq_handler(const registers* r)
{
    // register_to_serial(r);
    // serial_write_string("IRQ: ");
    const auto int_no = r->int_no;
    // serial_write_int(int_no);
    // serial_new_line();

    if (int_no >= 32)
    {
        switch (int_no - 32)
        {
        case 0:
            timer_handler();
            break;
        case 4:
            if (not ignore_4)
            {
                ignore_4 = true;
                serial_write_string("First instance");
                serial_write_string("Unhandled IRQ: ");
                serial_write_int(int_no);
                serial_new_line();
            }
            break;
        default:
            serial_write_string("Unhandled IRQ: ");
            serial_write_int(int_no);
            serial_new_line();
            break;
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


void idt_set_descriptor(const u8 idt_index, void* isr_stub, const u8 flags)
{
    idt_entry_t* descriptor = &idt_entries[idt_index];

    descriptor->isr_low = reinterpret_cast<u32>(isr_stub) & 0xFFFF;
    descriptor->kernel_cs = KERNEL_CS;
    // this value can be whatever offset your kernel code selector is in your GDT.
    // My entry point is 0x001005e0 so the offset is 0x0010(XXXX) (because of GRUB)
    descriptor->attributes = flags;
    descriptor->isr_high = reinterpret_cast<u32>(isr_stub) >> 16;
    descriptor->reserved = 0;
}


void idt_install()
{
    /* also installs irq */
    // serial_write_string("Remapping irq\n");
    // pic_irq_remap();
    // pic_disable();

    idt_pointer.limit = (sizeof(idt_entry_t) * 48) - 1;
    idt_pointer.base = reinterpret_cast<uintptr_t>(&idt_entries[0]); // this should point to first idt


    for (u8 idt_index = 0; idt_index < 48; idt_index++)
    {
        idt_set_descriptor(idt_index, isr_stub_table[idt_index], 0x8E);
        idt_vectors[idt_index] = true;
    }

    serial_write_string("Setting IDT base and limit. ");
    serial_write_string("Base: ");
    serial_write_hex(idt_pointer.base);
    serial_write_string(" Limit: ");
    serial_write_hex(idt_pointer.limit);
    serial_new_line();
    __asm__ volatile ("lidt %0" : : "m"(idt_pointer)); // load the new IDT
    serial_write_string("LDT has been set\n");
    __asm__ volatile ("sti"); // set the interrupt flag
    serial_write_string("Interrupts enabled\n");
}
