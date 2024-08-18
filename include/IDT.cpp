//
// Created by artypoole on 02/07/24.
//

#include "IDT.h"

#include "LocalAPIC.h"

#include "PIT.h"
#include "RTC.h"
#include "EventQueue.h"
#include "Serial.h"
#include "ports.h"
#include <stdint.h>


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
static bool idt_vectors[IDT_STUB_COUNT];
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
    WRITE("int_no, err_code: ");
    log.newLine();
    WRITE(r->int_no, true);
    WRITE(", ");
    WRITE(r->err_code, true);
    log.newLine();

    WRITE("gs, fs, es, ds: ");
    log.newLine();
    WRITE(r->gs, true);
    WRITE(", ");
    WRITE(r->fs, true);
    WRITE(", ");
    WRITE(r->es, true);
    WRITE(", ");
    WRITE(r->ds, true);
    log.newLine();

    WRITE("edi, esi, ebp, esp, ebx, edx, ecx, eax;");
    log.newLine();
    WRITE(r->edi, true);
    WRITE(", ");
    WRITE(r->esi, true);
    WRITE(", ");
    WRITE(r->ebp, true);
    WRITE(", ");
    WRITE(r->esp, true);
    WRITE(", ");
    WRITE(r->ebx, true);
    WRITE(", ");
    WRITE(r->edx, true);
    // WRITE(", ");
    // log.write_hex(r->ecx, true);
    // WRITE(", ");
    // WRITE(r->eax, true);
    log.newLine();

    WRITE("eip, cs, eflags, useresp, ss;");
    log.newLine();
    WRITE(r->eip, true);
    WRITE(", ");
    WRITE(r->cs, true);
    WRITE(", ");
    WRITE(r->eflags, true);
    WRITE(", ");
    WRITE(r->useresp, true);
    WRITE(", ");
    WRITE(r->ss, true);
    log.newLine();
}

void handle_div_by_zero(const cpu_registers_t* r)
{
    WRITE("Div by zero not handled. oops.\n");
    register_to_serial(r);
}


extern "C"
void exception_handler(const cpu_registers_t* r)
{
    auto &log = Serial::get();
    register_to_serial(r);

    WRITE("Exception: ");
    // log.write_hex(r->int_no, 4);
    // log.new_line();

    if (r->int_no < 32)
    {
        /* Display the description for the Exception that occurred.
        *  In this tutorial, we will simply halt the system using an
        *  infinite loop */
        WRITE(exception_messages[r->int_no]);
        log.newLine();
        switch (r->int_no)
        {
        case 0:
            // handle_div_by_zero(r);
            return;
        default:
            WRITE("Unhandled exception. System Halted!");
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
        2 	Cascade (used internally by the two PICs. never raised) or timer redirect in APIC
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
        14 	Primary ATA Bus
        15 	Secondary ATA Bus
        240-32  Spurious APIC
    */
    // register_to_serial(r);

    const auto int_no = r->int_no;

    if (int_no >= 32)
    {

        switch (int_no - 32)
        {
        case 0:
            pit_handler();
            break;
        case 1:
            keyboardHandler();
            break;
        case 4:
            break;
        case 8:
            rtc_handler();
            break;
        case 208:
            LOG("Spurious Interrupt");
            break;
        default:
            LOG("unhandled IRQ: ", int_no);
            break;
        }
    }

    // /* If the IDT entry that was invoked was greater than 40
    // *  (meaning IRQ8 - 15), then we need to send an EOI to
    // *  the slave controller */
    // if (int_no >= 40)
    // {
    //     outb(PIC2_COMMAND, PIC_EOI);
    // }
    //
    // /* In either case, we need to send an EOI to the master
    // *  interrupt controller too */
    // outb(PIC1_COMMAND, PIC_EOI);
    LAPIC_EOI();
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
    LOG("Initialising IDT");
    idt_pointer.limit = (sizeof(idt_entry_t) * IDT_STUB_COUNT) - 1;
    idt_pointer.base = reinterpret_cast<uintptr_t>(&idt_entries[0]); // this should point to first idt


    for (u8 idt_index = 0; idt_index < IDT_STUB_COUNT; idt_index++)
    {
        _setDescriptor(idt_index, isr_stub_table[idt_index], 0x8E);
        idt_vectors[idt_index] = true;
    }
    log.time_stamp();
    WRITE("\tSetting IDT base and limit. ");
    WRITE("Base: ");
    WRITE(idt_pointer.base, true);
    WRITE(" Limit: ");
    WRITE(idt_pointer.limit, true);
    NEWLINE();
    __asm__ volatile ("lidt %0" : : "m"(idt_pointer)); // load the new IDT
    LOG("IDT has been set");
    enable_interrupts();
    LOG("IDT initialised");
}
