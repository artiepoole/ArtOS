//
// Created by artypoole on 18/08/24.
//

#ifndef IOAPIC_H
#define IOAPIC_H

#include "types.h"
#include "APIC.h"

struct io_redirect_entry
{
    union
    {
        struct
        {
            LVT_entry lvt;
            u64 reserved : 23;
            u64 destination : 8;
        };

        struct
        {
            u32 lower;
            u32 upper;
        };
    };
};

class IOAPIC
{
public:
    IOAPIC(uintptr_t io_apic_physical_address);
    void pause();
    void resume();
    void remapIRQ(u8 irq_before, u8 irq_after);
    void disableIRQ(u8 irq_before);
    void enableAll();

private:
    uintptr_t volatile * base_addr;
    uintptr_t volatile * data_addr;
    io_redirect_entry redirect_entries[23];
};



#endif //IOAPIC_H
