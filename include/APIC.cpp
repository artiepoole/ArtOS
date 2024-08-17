//
// Created by artypoole on 16/08/24.
//

#include "APIC.h"

#include "ACPI.h"
#include "IDT.h"
#include "Serial.h"
#include "string.h"


// LVT entry offsets
#define SPURIOUS_OFFSET_VECTOR 0xF0
#define TIMER_LVT_OFFSET 0x320
#define THERMAL_LVT_OFFSET 0x330
#define PERFORMANCE_LVT_OFFSET 0x340
#define LINT0_LVT_OFFSET 0x350
#define LINT1_LVT_OFFSET 0x360
#define ERROR_LVT_OFFSET 0x370

// other local apic offsets
#define TIMER_INITIAL_COUNT 0x380
#define TIMER_DIVISOR_OFFSET 0x3E0
#define LOCAL_APIC_ID_OFFSET 0x20
#define EOI_OFFSET 0xB0

// io apic offsets
#define IOAPICID 0x00
#define IOAPICVER 0x01
#define IOAPICARB 0x02
#define IOREDTBL 03h // up to 3fh
#define IO_WRITE_OFFSET 0x00
#define IO_READ_OFFSET 0x10


#define IA32_APIC_BASE_MSR 0x1B
#define IA32_APIC_BASE_MSR_ENABLE 0x800


void cpuGetMSR(u32 msr, u32* lo, u32* hi)
{
    asm volatile("rdmsr" : "=a"(*lo), "=d"(*hi) : "c"(msr));
}

void cpuSetMSR(u32 msr, u32 lo, u32 hi)
{
    asm volatile("wrmsr" : : "a"(lo), "d"(hi), "c"(msr));
}

u32 cpu_get_apic_base()
{
    u32 eax, edx;
    cpuGetMSR(IA32_APIC_BASE_MSR, &eax, &edx);


    return (eax & 0xfffff000);
}

void cpu_set_apic_base(u32 apic)
{
    u32 edx = 0;
    u32 eax = (apic & 0xfffff0000) | IA32_APIC_BASE_MSR_ENABLE;

    cpuSetMSR(IA32_APIC_BASE_MSR, eax, edx);
}

void write_reg(u32 addr, u32 val)
{
    *(volatile u32*)addr = val;
}

u32 read_reg(u32 addr)
{
    return *reinterpret_cast<u32*>(addr);
}

u32* eoi_addr;

LocalAPIC::LocalAPIC(u32 local_apic_physical_address)
{
    base = reinterpret_cast<u32*>(local_apic_physical_address);
    eoi_addr = reinterpret_cast<u32*>(local_apic_physical_address + EOI_OFFSET);
    spurious = *reinterpret_cast<LVT_spurious_vector*>(local_apic_physical_address + SPURIOUS_OFFSET_VECTOR);

    // todo: troubleshoot not being able to set the value of the spurious vector here.
    full_lvt.timer = *reinterpret_cast<LVT_timer_entry*>(local_apic_physical_address + TIMER_LVT_OFFSET);
    full_lvt.thermal = *reinterpret_cast<LVT_entry*>(local_apic_physical_address + THERMAL_LVT_OFFSET);
    full_lvt.performance = *reinterpret_cast<LVT_entry*>(local_apic_physical_address + PERFORMANCE_LVT_OFFSET);
    full_lvt.LINT0 = *reinterpret_cast<LVT_entry*>(local_apic_physical_address + LINT0_LVT_OFFSET);
    full_lvt.LINT1 = *reinterpret_cast<LVT_entry*>(local_apic_physical_address + PERFORMANCE_LVT_OFFSET);
    full_lvt.error = *reinterpret_cast<LVT_entry*>(local_apic_physical_address + ERROR_LVT_OFFSET);
}

void LocalAPIC::configure_timer(const u32 hz)
{
    //TODO: implement https://github.com/dreamportdev/Osdev-Notes/blob/master/02_Architecture/08_Timers.md
}

void LAPIC_EOI()
{
    eoi_addr[0] = 0;
}

struct io_reg_select
{
    u8 apic_register;
    u32 reserved : 24;
};



// PIT is moved to 2 be default in APIC.
u8 before[3] = {2, 1, 8}; // PIT, keyboard, RTC
u8 after[3] = {0 + 32, 1 + 32, 8 + 32};

IOAPIC::IOAPIC(u32 io_apic_physical_address)
{
    base_addr = reinterpret_cast<u32*>(io_apic_physical_address);
    data_addr = reinterpret_cast<u32*>(io_apic_physical_address + 0x10);
}



void IOAPIC::pause()
{
}

void IOAPIC::resume()
{
}

// Also un masks
void IOAPIC::remapIRQ(const u8 irq_before, const u8 irq_after)
{
    io_redirect_entry data{};
    // load the previous entry, ensuring it is populated.
    *base_addr = (0x10 + irq_before*2);
    data.lower = *data_addr;
    *base_addr = (0x10 + irq_before*2+1);
    data.upper = *data_addr ;
    // change settings
    data.lvt.interrupt_mask = false;
    data.lvt.interrupt_vector = irq_after;
    // apply settings
    *base_addr = (0x10 + irq_before*2);
    *data_addr = data.lower;
    *base_addr = (0x10 +  irq_before*2+1);
    *data_addr = 0; // in our case, upper always 0
    redirect_entries[irq_before] = data;
}


void IOAPIC::disableIRQ(const u8 irq_before)
{
    if (redirect_entries[irq_before].lvt.interrupt_mask == true) return;

    io_redirect_entry data{};
    // load the previous entry, ensuring it is populated.
    *base_addr = (0x10 + irq_before*2);
    data.lower = *data_addr;
    *base_addr = (0x10 + irq_before*2+1);
    data.upper = *data_addr ;
    // change settings
    data.lvt.interrupt_mask = true;
    // write out data
    *base_addr = (0x10 + irq_before*2);
    *data_addr = data.lower;
    *base_addr = (0x10 +  irq_before*2+1);
    *data_addr = data.upper; // in our case, upper always 0
    // store for lookup
    redirect_entries[irq_before] = data;
}

void enableAll()
{
}
