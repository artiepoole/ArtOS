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

#include "IOAPIC.h"
#include "logging.h"
#include "paging.h"

#if FORLAPTOP
    #define TARGET_APIC 2
    #define LOGICAL_TARGET 2
    #define DESTINATION_MODE 1
#else
#define TARGET_APIC 0
#define LOGICAL_TARGET 0
#define DESTINATION_MODE 0
#endif

// io apic offsets
#define IOAPICID 0x00
#define IOAPICVER 0x01
#define IOAPICARB 0x02
#define IOREDTBL 03h // up to 3fh
#define IO_WRITE_OFFSET 0x00
#define IO_READ_OFFSET 0x10

#define IOREDTBL_START 0x10


IOAPIC::IOAPIC(uintptr_t io_apic_physical_address)
{
    paging_identity_map(io_apic_physical_address, 0x3F, true, false);
    base_addr = reinterpret_cast<uintptr_t*>(io_apic_physical_address);
    TIMESTAMP();
    WRITE("IOAPIC base addr: ");
    WRITE(io_apic_physical_address, true);
    NEWLINE();
    data_addr = reinterpret_cast<uintptr_t*>(io_apic_physical_address + IO_READ_OFFSET);
}


void IOAPIC::pause()
{
    // TODO: Not implemented
}

void IOAPIC::resume()
{
    // TODO: Not implemented
}

// Also un masks
void IOAPIC::remap_IRQ(const u8 irq_before, const u8 irq_after)
{
    io_redirect_entry data{};
    // load the previous entry, ensuring it is populated.
    *base_addr = (IOREDTBL_START + irq_before * 2);
    data.lower = *data_addr;
    *base_addr = (IOREDTBL_START + irq_before * 2 + 1);
    data.upper = *data_addr;
    // change settings
    data.lvt.interrupt_mask = false;
    data.lvt.interrupt_vector = irq_after;
    data.lvt.destination_mode = DESTINATION_MODE;
#if FORLAPTOP
    data.destination_lapic_addr = 0;
    data.destination_cluster_addr = 1;
#endif
    // apply settings
    *base_addr = (IOREDTBL_START + irq_before * 2);
    *data_addr = data.lower;
    *base_addr = (IOREDTBL_START + irq_before * 2 + 1);
    *data_addr = data.upper; // in our case, upper always 0
    redirect_entries[irq_before] = data;
    LOG("Remapped IRQ from ", irq_before, " to ", irq_after);
}


void IOAPIC::disable_IRQ(const u8 irq_before)
{
    if (redirect_entries[irq_before].lvt.interrupt_mask == true) return;

    io_redirect_entry data{};
    // load the previous entry, ensuring it is populated.
    *base_addr = (IOREDTBL_START + irq_before * 2);
    data.lower = *data_addr;
    *base_addr = (IOREDTBL_START + irq_before * 2 + 1);
    data.upper = *data_addr;
    // change settings
    data.lvt.interrupt_mask = true;
    // write out data
    *base_addr = (IOREDTBL_START + irq_before * 2);
    *data_addr = data.lower;
    *base_addr = (IOREDTBL_START + irq_before * 2 + 1);
    *data_addr = data.upper; // in our case, upper always 0
    // store for lookup
    redirect_entries[irq_before] = data;
}


void IOAPIC::enable_IRQ(const u8 irq_before)
{
    if (redirect_entries[irq_before].lvt.interrupt_mask == false) return;

    io_redirect_entry data{};
    // load the previous entry, ensuring it is populated.
    *base_addr = (IOREDTBL_START + irq_before * 2);
    data.lower = *data_addr;
    *base_addr = (IOREDTBL_START + irq_before * 2 + 1);
    data.upper = *data_addr;
    // change settings
    data.lvt.interrupt_mask = false;
    // write out data
    *base_addr = (IOREDTBL_START + irq_before * 2);
    *data_addr = data.lower;
    *base_addr = (IOREDTBL_START + irq_before * 2 + 1);
    *data_addr = data.upper; // in our case, upper always 0
    // store for lookup
    redirect_entries[irq_before] = data;
}
