//
// Created by artypoole on 02/07/24.
//

#include "idt.h"

bool ignore_4 = false;

u16 KERNEL_CS = 0x0010;
u16 KERNEL_DS = 0x0018;
void register_to_serial(const registers* r)
{
    log.write_string("int_no, err_code: ");
    log.new_line();
    log.write_hex(r->int_no);
    log.write_string(", ");
    log.write_hex(r->err_code);
    log.new_line();

    log.write_string("gs, fs, es, ds: ");
    log.new_line();
    log.write_hex(r->gs);
    log.write_string(", ");
    log.write_hex(r->fs);
    log.write_string(", ");
    log.write_hex(r->es);
    log.write_string(", ");
    log.write_hex(r->ds);
    log.new_line();

    log.write_string("edi, esi, ebp, esp, ebx, edx, ecx, eax;");
    log.new_line();
    log.write_hex(r->edi);
    log.write_string(", ");
    log.write_hex(r->esi);
    log.write_string(", ");
    log.write_hex(r->ebp);
    log.write_string(", ");
    log.write_hex(r->esp);
    log.write_string(", ");
    log.write_hex(r->ebx);
    log.write_string(", ");
    log.write_hex(r->edx);
    // log.write_string(", ");
    // log.write_hex(r->ecx, 4);
    // log.write_string(", ");
    // log.write_hex(r->eax, 4);
    log.new_line();

    log.write_string("eip, cs, eflags, useresp, ss;");
    log.new_line();
    log.write_hex(r->eip);
    log.write_string(", ");
    log.write_hex(r->cs);
    log.write_string(", ");
    log.write_hex(r->eflags);
    log.write_string(", ");
    log.write_hex(r->useresp);
    log.write_string(", ");
    log.write_hex(r->ss);
    log.new_line();
}

// __attribute__((noreturn))
void handle_div_by_zero(const registers* r)
{
    log.write_string("Div by zero not handled. oops.\n");
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

    log.write_string("Exception: ");
    // log.write_hex(r->int_no, 4);
    // log.new_line();

    if (r->int_no < 32)
    {
        /* Display the description for the Exception that occurred.
        *  In this tutorial, we will simply halt the system using an
        *  infinite loop */
        log.write_string(exception_messages[r->int_no]);
        log.new_line();
        switch (r->int_no)
        {
        case 0:
            // handle_div_by_zero(r);
            return;
        default:
            log.write_string("Unhandled exception. System Halted!");
            for (;;);
        }
    }
    else
    {
        log.write_string("Unknown exception. System Halted!\n");
        for (;;);
    }
}

extern "C"
void irq_handler(const registers* r)
{
    // register_to_serial(r);
    // log.write_string("IRQ: ");
    const auto int_no = r->int_no;
    // log.write_int(int_no);
    // log.new_line();

    if (int_no >= 32)
    {
        switch (int_no - 32)
        {
        case 0:
            timer_handler();
            break;
        case 4:
            log.write_string("First instance");
            log.write_string("Unhandled IRQ: ");
            log.write_int(int_no);
            log.new_line();
            break;
        default:
            log.write_string("Unhandled IRQ: ");
            log.write_int(int_no);
            log.new_line();
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


void IDT::set_descriptor(const u8 idt_index, void* isr_stub, const u8 flags)
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


IDT::IDT()
{
    /* also installs irq */
    // log.write_string("Remapping irq\n");
    // pic_irq_remap();
    // pic_disable();
    log.write_string("IDT installed\n");
    idt_pointer.limit = (sizeof(idt_entry_t) * 48) - 1;
    idt_pointer.base = reinterpret_cast<uintptr_t>(&idt_entries[0]); // this should point to first idt


    for (u8 idt_index = 0; idt_index < 48; idt_index++)
    {
        set_descriptor(idt_index, isr_stub_table[idt_index], 0x8E);
        idt_vectors[idt_index] = true;
    }

    log.write_string("Setting IDT base and limit. ");
    log.write_string("Base: ");
    log.write_hex(idt_pointer.base);
    log.write_string(" Limit: ");
    log.write_hex(idt_pointer.limit);
    log.new_line();
    __asm__ volatile ("lidt %0" : : "m"(idt_pointer)); // load the new IDT
    log.write_string("LDT has been set\n");
    __asm__ volatile ("sti"); // set the interrupt flag
    log.write_string("Interrupts enabled\n");
}
