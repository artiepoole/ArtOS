//
// Created by artypoole on 18/08/24.
//

#ifndef IOAPIC_H
#define IOAPIC_H

#include "types.h"
#include "APIC.h"


union io_redirect_entry
{
    struct
    {
        LVT_entry lvt;
        u32 reserved : 24;
        u32 destination_lapic_addr : 4;
        u32 destination_cluster_addr : 4;
    };

    struct
    {
        u32 lower;
        u32 upper;
    };
};


class IOAPIC
{
public:
    IOAPIC(uintptr_t io_apic_physical_address);
    void pause();
    void resume();
    void remap_IRQ(u8 irq_before, u8 irq_after);
    void disable_IRQ(u8 irq_before);
    void enable_all();

private:
    uintptr_t volatile* base_addr;
    uintptr_t volatile* data_addr;
    io_redirect_entry redirect_entries[23];
};


#endif //IOAPIC_H
