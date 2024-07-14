//
// Created by artypoole on 02/07/24.
//

#include "IDT.h"

// todo: move some of this stuff to an "interrupts.cpp" or similar.
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
    "div by zero", // 0
    "debug exception", // 1
    "non-maskable interrupt exception", // 2
    "breakpoint exception", // 3
    "into detected overflow exception", // 4
    "out of bounds exception", // 5
    "invalid opcode exception", // 6
    "double fault exception", // 7
    "coprocessor segment overrun exception", // 8
    "bad TSS exception", // 9
    "segment not present exception", // 10
    "stack fault exception", // 11
    "general protection fault exception", // 12
    "page fault exception", // 13
    "unknown interrupt exception", // 14
    "coprocessor fault exception", // 15
    "alignment check exception", // 16
    "machine check exception", // 17
    "reserved exceptions", // 18
    "reserved exceptions", // 19
    "reserved exceptions", // 20
    "reserved exceptions", // 21
    "reserved exceptions", // 22
    "reserved exceptions", // 23
    "reserved exceptions", // 24
    "reserved exceptions", // 25
    "reserved exceptions", // 26
    "reserved exceptions", // 27
    "reserved exceptions", // 28
    "reserved exceptions", // 29
    "reserved exceptions", // 30
    "reserved exceptions", // 31
};

void register_to_serial(const cpu_registers_t* r)
{
    auto &log = Serial::get();
    log.write("int_no, err_code: ");
    log.newLine();
    log.write(r->int_no, true);
    log.write(", ");
    log.write(r->err_code, true);
    log.newLine();

    log.write("gs, fs, es, ds: ");
    log.newLine();
    log.write(r->gs, true);
    log.write(", ");
    log.write(r->fs, true);
    log.write(", ");
    log.write(r->es, true);
    log.write(", ");
    log.write(r->ds, true);
    log.newLine();

    log.write("edi, esi, ebp, esp, ebx, edx, ecx, eax;");
    log.newLine();
    log.write(r->edi, true);
    log.write(", ");
    log.write(r->esi, true);
    log.write(", ");
    log.write(r->ebp, true);
    log.write(", ");
    log.write(r->esp, true);
    log.write(", ");
    log.write(r->ebx, true);
    log.write(", ");
    log.write(r->edx, true);
    // log.write(", ");
    // log.write_hex(r->ecx, true);
    // log.write(", ");
    // log.write(r->eax, true);
    log.newLine();

    log.write("eip, cs, eflags, useresp, ss;");
    log.newLine();
    log.write(r->eip, true);
    log.write(", ");
    log.write(r->cs, true);
    log.write(", ");
    log.write(r->eflags, true);
    log.write(", ");
    log.write(r->useresp, true);
    log.write(", ");
    log.write(r->ss, true);
    log.newLine();
}

void handle_div_by_zero(const cpu_registers_t* r)
{
    auto &log = Serial::get();
    log.write("Div by zero not handled. oops.\n");
    register_to_serial(r);
}


extern "C"
void exception_handler(const cpu_registers_t* r)
{
    auto &log = Serial::get();
    register_to_serial(r);

    log.write("Exception: ");
    // log.write_hex(r->int_no, 4);
    // log.new_line();

    if (r->int_no < 32)
    {
        /* Display the description for the Exception that occurred.
        *  In this tutorial, we will simply halt the system using an
        *  infinite loop */
        log.write(exception_messages[r->int_no]);
        log.newLine();
        switch (r->int_no)
        {
        case 0:
            // handle_div_by_zero(r);
            return;
        default:
            log.write("Unhandled exception. System Halted!");
            for (;;);
        }
    }
}

extern "C"
void irq_handler(const cpu_registers_t* r)
{
    /*
        0 	Programmable Interrupt Timer Interrupt
        1 	Keyboard Interrupt
        2 	Cascade (used internally by the two PICs. never raised)
        3 	COM2 (if enabled)
        4 	COM1 (if enabled)
        5 	LPT2 (if enabled)
        6 	Floppy Disk
        7 	LPT1 / Unreliable "spurious" interrupt (usually)
        8 	CMOS real-time clock (if enabled)
        9 	Free for peripherals / legacy SCSI / NIC
        10 	Free for peripherals / SCSI / NIC
        11 	Free for peripherals / SCSI / NIC
        12 	PS2 Mouse
        13 	FPU / Coprocessor / Inter-processor
        14 	Primary ATA Hard Disk
        15 	Secondary ATA Hard Disk
    */
    auto &log = Serial::get();
    // register_to_serial(r);

    const auto int_no = r->int_no;
    // log.log("IRQ: ",int_no);

    if (int_no >= 32)
    {
        switch (int_no - 32)
        {
        case 0:
            timerHandler();
            break;
        case 1:
            keyboardHandler();
            break;
        case 4:
            break;
        case 8:
            RTCHandler();
            break;
        default:
            log.write("Unhandled IRQ: ");
            log.write(int_no);
            log.newLine();
            break;
        }
    }

    /* If the IDT entry that was invoked was greater than 40
    *  (meaning IRQ8 - 15), then we need to send an EOI to
    *  the slave controller */
    if (int_no >= 40)
    {
        outb(PIC2_COMMAND, PIC_EOI);
    }

    /* In either case, we need to send an EOI to the master
    *  interrupt controller too */
    outb(PIC1_COMMAND, PIC_EOI);
}


void IDT::_setDescriptor(const u8 idt_index, void* isr_stub, const u8 flags)
{
    idt_entry_t* descriptor = &idt_entries[idt_index];

    descriptor->isr_low = reinterpret_cast<u32>(isr_stub) & 0xFFFF;
    descriptor->kernel_cs = KERNEL_CS;
    // this value can be whatever offset your sys code selector is in your GDT.
    // My entry point is 0x001005e0 so the offset is 0x0010(XXXX) (because of GRUB)
    descriptor->attributes = flags;
    descriptor->isr_high = reinterpret_cast<u32>(isr_stub) >> 16;
    descriptor->reserved = 0;
}


IDT::IDT()
{
    auto &log = Serial::get();
    log.log("Initialising IDT");
    idt_pointer.limit = (sizeof(idt_entry_t) * 48) - 1;
    idt_pointer.base = reinterpret_cast<uintptr_t>(&idt_entries[0]); // this should point to first idt


    for (u8 idt_index = 0; idt_index < 48; idt_index++)
    {
        _setDescriptor(idt_index, isr_stub_table[idt_index], 0x8E);
        idt_vectors[idt_index] = true;
    }

    log.write("\tSetting IDT base and limit. ");
    log.write("Base: ");
    log.write(idt_pointer.base, true);
    log.write(" Limit: ");
    log.write(idt_pointer.limit, true);
    log.newLine();
    __asm__ volatile ("lidt %0" : : "m"(idt_pointer)); // load the new IDT
    log.write("\tIDT has been set\n");
    enable_interrupts();
    log.log("IDT initialised");
}
