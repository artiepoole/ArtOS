// ArtOS - hobby operating system by Artie Poole
// Copyright (C) 2025 Stuart Forbes Poole <artiepoole>
//
//     This program is free software: you can redistribute it and/or modify
//     it under the terms of the GNU General Public License as published by
//     the Free Software Foundation, either version 3 of the License, or
//     (at your option) any later version.
//
//     This program is distributed in the hope that it will be useful,
//     but WITHOUT ANY WARRANTY; without even the implied warranty of
//     MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//     GNU General Public License for more details.
//
//     You should have received a copy of the GNU General Public License
//     along with this program.  If not, see <https://www.gnu.org/licenses/>

//
// Created by artypoole on 18/08/24.
//

#include "LocalAPIC.h"

#include <CPUID.h>
#include <TSC.h>

#include "logging.h"
#include "paging.h"
#include "IDT.h"
uintptr_t* eoi_addr;

// LVT entry offsets
#define SPURIOUS_OFFSET_VECTOR 0xF0
#define TIMER_LVT_OFFSET 0x320
#define THERMAL_LVT_OFFSET 0x330
#define PERFORMANCE_LVT_OFFSET 0x340
#define LINT0_LVT_OFFSET 0x350
#define LINT1_LVT_OFFSET 0x360
#define ERROR_LVT_OFFSET 0x370

// other local apic offsets
#define TIMER_INITIAL_COUNT_OFFSET 0x380
#define TIMER_CURRENT_COUNT_OFFSET 0x390
#define TIMER_DIVISOR_OFFSET 0x3E0
#define LOCAL_APIC_ID_OFFSET 0x20
#define EOI_OFFSET 0xB0
#define LDR_OFFSET 0xD0
#define DFR_OFFSET 0xE0
#define ID_OFFSET 0x20

union destination_format_register
{
    struct
    {
        u32 pad0 : 28;
        u32 model : 4;
    };

    u32 raw;
}__attribute__((packed));


union local_destination_register
{
    struct
    {
        u32 pad0 : 8;
        u32 pad1 : 16;
        u32 lapic_addr : 4;
        u32 cluster_addr : 4;
    };

    u32 raw;
}__attribute__((packed));


volatile bool calibrated = false;

LocalAPIC::LocalAPIC(uintptr_t local_apic_physical_address)
{
    paging_identity_map(local_apic_physical_address, 0x3f0, true, false);
    base = local_apic_physical_address;
    TIMESTAMP();
    WRITE("LAPIC base addr: ");
    WRITE(local_apic_physical_address, true);
    NEWLINE();
    eoi_addr = reinterpret_cast<uintptr_t*>(base + EOI_OFFSET);
    auto spv_addr = reinterpret_cast<u32*>(base + SPURIOUS_OFFSET_VECTOR);
    [[maybe_unused]] auto id = *reinterpret_cast<u32*>(base + ID_OFFSET);
    LOG("LAPIC id: ", id);
    // Writing to registers must be done using a 32-bit write. This means that you cannot vary the members using a pointer obj
    // We take a copy, edit the copy and write the entire 32-bit copy to the original address and store the new register.
    auto local_spurious = *reinterpret_cast<LVT_spurious_vector*>(spv_addr);
    local_spurious.spurious_vector = 0xF0;
    local_spurious.software_enable = true;
    *spv_addr = local_spurious.raw;

    spurious_vector_entry = reinterpret_cast<LVT_spurious_vector*>(spv_addr);
    LOG(
        "Spurious vector set. Spurious vector: ",
        static_cast<u8>(spurious_vector_entry->raw & 0xFF), // vector is only bottom 8 bits. This cast is very pedantic.
        " Software enabled: ",
        static_cast<bool>(local_spurious.software_enable));

    full_lvt.timer = *reinterpret_cast<LVT_timer_entry*>(base + TIMER_LVT_OFFSET);
    full_lvt.thermal = *reinterpret_cast<LVT_entry*>(base + THERMAL_LVT_OFFSET);
    full_lvt.performance = *reinterpret_cast<LVT_entry*>(base + PERFORMANCE_LVT_OFFSET);
    full_lvt.LINT0 = *reinterpret_cast<LVT_entry*>(base + LINT0_LVT_OFFSET);
    full_lvt.LINT1 = *reinterpret_cast<LVT_entry*>(base + PERFORMANCE_LVT_OFFSET);
    full_lvt.error = *reinterpret_cast<LVT_entry*>(base + ERROR_LVT_OFFSET);

    auto volatile model = reinterpret_cast<u32*>(base + DFR_OFFSET);
    *model = 0xFFFFFFFF;
    [[maybe_unused]] auto dfr = reinterpret_cast<destination_format_register*>(base + DFR_OFFSET);
    LOG("LAPIC DFR mode: ", static_cast<u8>(dfr->model));

    local_destination_register local_ldr{};
    auto ldr_addr = reinterpret_cast<u32*>(base + LDR_OFFSET);
    local_ldr.lapic_addr = 0;
    local_ldr.cluster_addr = 1;
    *ldr_addr = local_ldr.raw;
    [[maybe_unused]] auto volatile* ldr = reinterpret_cast<local_destination_register*>(base + LDR_OFFSET);
    LOG(
        "LAPIC LDR cluster address set: ",
        static_cast<u8>(ldr->cluster_addr)
    );
}

// Returns number of clock cycles per LAPIC tick (e.g. if divisor set to 128, this will be LAPIC_clock_rate/128. Measured using 1000 ticks. Set divisor first).
void LocalAPIC::calibrate_timer()
{
    constexpr u32 n_LAPIC_ticks = 0x10000;
    const u64 tsc_start = TSC_get_ticks();

    // Start LAPIC timer with a known tick count
    *reinterpret_cast<volatile u32*>(base + TIMER_INITIAL_COUNT_OFFSET) = n_LAPIC_ticks;

    // Wait until your interrupt handler sets `calibrated = true`
    while (!calibrated) { }

    const u64 tsc_end = TSC_get_ticks();
    const u64 elapsed_tsc = tsc_end - tsc_start;

    const u64 tsc_freq = cpuid_get_TSC_frequency(); // in Hz

    LAPIC_rate = static_cast<u64>(n_LAPIC_ticks) * tsc_freq / elapsed_tsc;
    LOG("LAPIC calibrated:");
    LOG("  TSC elapsed cycles: ", elapsed_tsc);
    LOG("  TSC frequency: ", tsc_freq);
    LOG("  LAPIC rate (Hz): ", LAPIC_rate);
}

bool LocalAPIC::ready() const
{
    return is_ready;
}

int LocalAPIC::start_timer_ms(u32 ms) const
{
    if (!is_ready) return -1;
    const u64 n_LAPIC_ticks = static_cast<u64>(ms) * LAPIC_rate / 1000;
    if (n_LAPIC_ticks == 0 || n_LAPIC_ticks > UINT32_MAX) return -1;
    *reinterpret_cast<u32*>(base + TIMER_INITIAL_COUNT_OFFSET) = static_cast<u32>(n_LAPIC_ticks);
    return static_cast<int>(n_LAPIC_ticks);
}

int LocalAPIC::start_timer_us(u32 us) const
{
    if (!is_ready) return -1;
    const u64 n_LAPIC_ticks = static_cast<u64>(us) * LAPIC_rate / 1000000;
    if (n_LAPIC_ticks == 0 || n_LAPIC_ticks > UINT32_MAX) return -1;
    *reinterpret_cast<u32*>(base + TIMER_INITIAL_COUNT_OFFSET) = static_cast<u32>(n_LAPIC_ticks);
    return static_cast<int>(n_LAPIC_ticks);
}

/* DEPENDS ON PIT and IDT */
void LocalAPIC::configure_timer(const DIVISOR divisor)
{
    // TODO: handle incorrect divisors
    full_lvt.timer.parts.interrupt_vector = LAPIC_CALIBRATE_IRQ + 32;
    full_lvt.timer.parts.timer_mode = 0;
    full_lvt.timer.parts.interrupt_mask = 0;


    *reinterpret_cast<u32*>(base + TIMER_LVT_OFFSET) = full_lvt.timer.raw;

    auto timer = *reinterpret_cast<LVT_timer_entry*>(base + TIMER_LVT_OFFSET);

    // TODO: this just uses 128
    *reinterpret_cast<u32*>(base + TIMER_DIVISOR_OFFSET) = static_cast<u32>(divisor);

    calibrate_timer();

    full_lvt.timer.parts.interrupt_vector = LAPIC_IRQ + 32;
    *reinterpret_cast<u32*>(base + TIMER_LVT_OFFSET) = full_lvt.timer.raw;

    is_ready = true;
}

void LAPIC_EOI()
{
    eoi_addr[0] = 0;
}


void LAPIC_calibrate_handler()
{
    LOG("LAPIC CALIBRATE INTERRUPT");
    calibrated = true;
}
