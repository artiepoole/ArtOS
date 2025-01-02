//
// Created by artypoole on 02/07/24.
//

#include "IDT.h"

#include <GDT.h>

#include "IDE_handler.h"

#include "LocalAPIC.h"

#include "PIT.h"
#include "RTC.h"
#include "EventQueue.h"
#include "logging.h"
#include "stdint.h"


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




inline constexpr char exception_messages[][40] =
{
    "div by zero", // 0
    "debug exception", // 1
    "non-maskable interrupt exception", // 2
    "breakpoint exception", // 3
    "into detected overflow exception", // 4
    "out of bounds exception", // 5
    "invalid opcode exception", // 6
    "device not available exception", // 7
    "double fault exception", // 8
    "coprocessor segment overrun exception", // 9
    "bad TSS exception", // 10
    "segment not present exception", // 11
    "stack fault exception", // 12
    "general protection fault exception", // 13
    "page fault exception", // 14
    "unknown interrupt exception", // 15
    "coprocessor fault exception", // 16
    "alignment check exception", // 17
    "machine check exception", // 18
    "SIMD floating point exception", // 19
    "reserved exceptions", //
    "reserved exceptions", //
    "reserved exceptions", //
    "reserved exceptions", //
    "reserved exceptions", //
    "reserved exceptions", //
    "reserved exceptions", //
    "reserved exceptions", //
    "reserved exceptions", //
    "reserved exceptions", //
    "reserved exceptions", //
    "reserved exceptions", //
};

void log_registers([[maybe_unused]] const cpu_registers_t* r)
{
    WRITE("int_no, err_code: ");
    NEWLINE();
    WRITE(r->int_no, true);
    WRITE(", ");
    WRITE(r->err_code, true);
    NEWLINE();

    WRITE("gs, fs, es, ds: ");
    NEWLINE();
    WRITE(r->gs, true);
    WRITE(", ");
    WRITE(r->fs, true);
    WRITE(", ");
    WRITE(r->es, true);
    WRITE(", ");
    WRITE(r->ds, true);
    NEWLINE();

    WRITE("edi, esi, ebp, esp, ebx, edx, ecx, eax;");
    NEWLINE();
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

    NEWLINE();

    WRITE("eip, cs, eflags, useresp, ss;");
    NEWLINE();
    WRITE(r->eip, true);
    WRITE(", ");
    WRITE(r->cs, true);
    WRITE(", ");
    WRITE(r->eflags, true);
    WRITE(", ");
    WRITE(r->useresp, true);
    WRITE(", ");
    WRITE(r->ss, true);
    NEWLINE();
}

void handle_div_by_zero(const cpu_registers_t* r)
{
    WRITE("Div by zero not handled. oops.\n");
    log_registers(r);
}


extern "C"
void exception_handler(const cpu_registers_t* r)
{
    log_registers(r);

    WRITE("Exception: ");


    if (r->int_no < 32)
    {
        /* Display the description for the Exception that occurred.
        *  In this tutorial, we will simply halt the system using an
        *  infinite loop */
        WRITE(exception_messages[r->int_no]);
        NEWLINE();
        switch (r->int_no)
        {
        case 0:
            return;
        default:
            WRITE("Unhandled exception. System Halted!");
            for (;;);
        }
    }
}

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
    16  LAPIC
    240-32  Spurious APIC
*/


extern "C"
void irq_handler(const cpu_registers_t* r)
{
    if (const auto int_no = r->int_no; int_no >= 32)
    {
        switch (int_no - 32)
        {
        case PIC_IRQ:
            pit_handler();
            break;
        case KEYBOARD_IRQ:
            keyboardHandler();
            break;
        case COM1_IRQ:
            break;
        case RTC_IRQ:
            rtc_handler();
            break;
        case IDE_PRIMARY_IRQ:
            IDE_handler(true);
            break;
        case IDE_SECONDARY_IRQ:
            IDE_handler(false);
            break;
        case LAPIC_IRQ:
            LAPIC_handler(r);
            break;
        case LAPIC_CALIBRATE_IRQ:
            LAPIC_calibrate_handler();
            break;
        case SPURIOUS_IRQ:
            LOG("Spurious Interrupt");
            break;
        default:
            LOG("unhandled IRQ: ", int_no);
            break;
        }
    }
    // Send end of interrupt.
    LAPIC_EOI();
}


void IDT::_setDescriptor(const u8 idt_index, void* isr_stub, const u8 flags)
{
    idt_entry_t* descriptor = &idt_entries[idt_index];

    descriptor->isr_low = reinterpret_cast<u32>(isr_stub) & 0xFFFF;
    descriptor->kernel_cs = kernel_cs_offset;

    // this value can be whatever offset your sys code selector is in your GDT.
    // My entry point is 0x001005e0 so the offset is 0x0010(XXXX) (because of GRUB)
    descriptor->attributes = flags;
    descriptor->isr_high = reinterpret_cast<u32>(isr_stub) >> 16;
    descriptor->reserved = 0;
}


IDT::IDT()
{

    LOG("Initialising IDT");
    idt_pointer.limit = (sizeof(idt_entry_t) * IDT_STUB_COUNT) - 1;
    idt_pointer.base = reinterpret_cast<uintptr_t>(&idt_entries[0]); // this should point to first idt


    for (u8 idt_index = 0; idt_index < IDT_STUB_COUNT; idt_index++)
    {
        _setDescriptor(idt_index, isr_stub_table[idt_index], 0x8E);
        idt_vectors[idt_index] = true;
    }
    TIMESTAMP();
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
