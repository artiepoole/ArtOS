//
// Created by artypoole on 02/07/24.
//

#include "IDT.h"

struct idt_entry_t
{
    u16 isr_low; // The lower 16 bits of the ISR's address
    u16 kernel_cs; // The GDT segment selector that the CPU will load into CS before calling the ISR
    u8 reserved; // Set to zero
    u8 attributes; // Type and attributes; see the IDT page
    u16 isr_high; // The higher 16 bits of the ISR's address
} __attribute__((packed));

struct idt_ptr_t
{
    u16 limit;
    u32 base;
} __attribute__((packed));

extern void* isr_stub_table[];
static bool idt_vectors[48];
static idt_ptr_t idt_pointer;
static idt_entry_t idt_entries[256]; // Create an array of IDT entries; aligned for performance

u16 KERNEL_CS = 0x0010;
u16 KERNEL_DS = 0x0018;


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
    "reserved exceptions",
    "reserved exceptions",
};

void register_to_serial(const registers* r)
{
    auto &log = Serial::get();
    log.writeString("int_no, err_code: ");
    log.newLine();
    log.writeHex(r->int_no);
    log.writeString(", ");
    log.writeHex(r->err_code);
    log.newLine();

    log.writeString("gs, fs, es, ds: ");
    log.newLine();
    log.writeHex(r->gs);
    log.writeString(", ");
    log.writeHex(r->fs);
    log.writeString(", ");
    log.writeHex(r->es);
    log.writeString(", ");
    log.writeHex(r->ds);
    log.newLine();

    log.writeString("edi, esi, ebp, esp, ebx, edx, ecx, eax;");
    log.newLine();
    log.writeHex(r->edi);
    log.writeString(", ");
    log.writeHex(r->esi);
    log.writeString(", ");
    log.writeHex(r->ebp);
    log.writeString(", ");
    log.writeHex(r->esp);
    log.writeString(", ");
    log.writeHex(r->ebx);
    log.writeString(", ");
    log.writeHex(r->edx);
    // log.writeString(", ");
    // log.write_hex(r->ecx, 4);
    // log.writeString(", ");
    // log.write_hex(r->eax, 4);
    log.newLine();

    log.writeString("eip, cs, eflags, useresp, ss;");
    log.newLine();
    log.writeHex(r->eip);
    log.writeString(", ");
    log.writeHex(r->cs);
    log.writeString(", ");
    log.writeHex(r->eflags);
    log.writeString(", ");
    log.writeHex(r->useresp);
    log.writeString(", ");
    log.writeHex(r->ss);
    log.newLine();
}

void handle_div_by_zero(const registers* r)
{
    auto &log = Serial::get();
    log.writeString("Div by zero not handled. oops.\n");
    register_to_serial(r);
}


extern "C"
void exception_handler(const registers* r)
{
    auto &log = Serial::get();
    register_to_serial(r);

    log.writeString("Exception: ");
    // log.write_hex(r->int_no, 4);
    // log.new_line();

    if (r->int_no < 32)
    {
        /* Display the description for the Exception that occurred.
        *  In this tutorial, we will simply halt the system using an
        *  infinite loop */
        log.writeString(exception_messages[r->int_no]);
        log.newLine();
        switch (r->int_no)
        {
        case 0:
            // handle_div_by_zero(r);
            return;
        default:
            log.writeString("Unhandled exception. System Halted!");
            for (;;);
        }
    }
}

extern "C"
void irq_handler(const registers* r)
{
    auto &log = Serial::get();
    // register_to_serial(r);
    // log.writeString("IRQ: ");
    const auto int_no = r->int_no;
    // log.write_int(int_no);
    // log.new_line();

    if (int_no >= 32)
    {
        switch (int_no - 32)
        {
        case 0:
            timerHandler();
            break;
        case 1:
            keyboard_handler();
            break;
        case 4:
            log.writeString("First instance");
            log.writeString("Unhandled IRQ: ");
            log.writeInt(int_no);
            log.newLine();
            break;
        default:
            log.writeString("Unhandled IRQ: ");
            log.writeInt(int_no);
            log.newLine();
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


void IDT::_setDescriptor(const u8 idt_index, void* isr_stub, const u8 flags)
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
    auto &log = Serial::get();
    /* also installs irq */
    // log.writeString("Remapping irq\n");
    // pic_irq_remap();
    // pic_disable();
    log.writeString("IDT installed\n");
    idt_pointer.limit = (sizeof(idt_entry_t) * 48) - 1;
    idt_pointer.base = reinterpret_cast<uintptr_t>(&idt_entries[0]); // this should point to first idt


    for (u8 idt_index = 0; idt_index < 48; idt_index++)
    {
        _setDescriptor(idt_index, isr_stub_table[idt_index], 0x8E);
        idt_vectors[idt_index] = true;
    }

    log.writeString("Setting IDT base and limit. ");
    log.writeString("Base: ");
    log.writeHex(idt_pointer.base);
    log.writeString(" Limit: ");
    log.writeHex(idt_pointer.limit);
    log.newLine();
    __asm__ volatile ("lidt %0" : : "m"(idt_pointer)); // load the new IDT
    log.writeString("LDT has been set\n");
    __asm__ volatile ("sti"); // set the interrupt flag
    log.writeString("Interrupts enabled\n");
}
