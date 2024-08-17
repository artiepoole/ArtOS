//
// Created by artypoole on 16/08/24.
//

#ifndef APIC_H
#define APIC_H
#include "types.h"


struct LVT_timer_entry
{
    union
    {
        struct
        {
            u8 interrupt_vector : 8;
            u8 pad0 : 4;
            bool delivery_status : 1; // 12
            u8 pad1 : 3;
            bool interrupt_mask : 1;
            u8 timer_mode : 2;
            u16 pad : 13;
        } parts;

        u32 entry;
    };
};

struct LVT_entry
{
    union
    {
        struct
        {
            u8 interrupt_vector : 8;
            u8 delivery_mode : 3;
            bool destination_mode : 1;
            bool delivery_status : 1;
            bool pin_polarity : 1;
            bool remote_IRR : 1;
            bool trigger_mode : 1;
            bool interrupt_mask : 1;
            u16 pad : 15;
        };

        u32 entry;
    };
};

struct LVT_spurious_vector
{
    union
    {
        struct
        {
            u8 spurious_vector : 8;
            bool software_enable : 1;
            bool focus_checking : 1;
            u32 reserved : 22;
        };

        u32 raw;
    };
};

struct LVT
{
    LVT_timer_entry timer;
    LVT_entry thermal;
    LVT_entry performance;
    LVT_entry LINT0;
    LVT_entry LINT1;
    LVT_entry error;
};

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


void LAPIC_EOI();

class LocalAPIC
{
public:
    LocalAPIC(u32 local_apic_physical_address);
    void configure_timer(u32 hz);

private:
    u32* base;

    LVT full_lvt;
    LVT_spurious_vector spurious;
};


class IOAPIC
{
public:
    IOAPIC(u32 io_apic_physical_address);
    void pause();
    void resume();
    void remapIRQ(u8 irq_before, u8 irq_after);
    void disableIRQ(u8 irq_before);
    void enableAll();

private:
    u32 volatile * base_addr;
    u32 volatile * data_addr;
    io_redirect_entry redirect_entries[23];
};


#endif //APIC_H
