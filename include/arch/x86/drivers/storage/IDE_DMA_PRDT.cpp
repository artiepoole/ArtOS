//
// Created by artypoole on 27/08/24.
//

#include "IDE_DMA_PRDT.h"

#include "logging.h"
#include "ports.h"

#include "stdlib.h"
// http://www.osdever.net/downloads/docs/idems100.zip
#define CMD_OFFSET 0x00
#define STATUS_OFFSET 0x02
#define PRDT_START_OFFSET  0x04


#define PRDT_SIZE 65536

u8 IDE_DMA_physical_region[PRDT_SIZE] __attribute__((aligned(1024 * 64))) = {};
PRDT_t IDE_DMA_prd_table{};



//  https://forum.osdev.org/viewtopic.php?t=19056
u8* DMA_init_PRDT(u16 base_port)
{
    IDE_DMA_prd_table.descriptor.base_addr = reinterpret_cast<u32>(IDE_DMA_physical_region) & 0xFFFFFFFE; // last bit reserved
    IDE_DMA_prd_table.descriptor.length_in_b = 0;
    IDE_DMA_prd_table.descriptor.end_of_table = 1;

    const auto table_loc = reinterpret_cast<u32>(&IDE_DMA_prd_table.descriptor) & 0xFFFFFFFC; // last 2 bits reserved

    outw(base_port + PRDT_START_OFFSET, table_loc & 0xFFFF); // low bytes
    outw(base_port + PRDT_START_OFFSET + 2, (table_loc >> 16) & 0xFFFF); // high bytes

    if (u8 top = inb(base_port + PRDT_START_OFFSET + 3); top != (table_loc >> 24 & 0xFF))
    {
        LOG("Error setting physical region");
        return nullptr;
    }
    return IDE_DMA_physical_region;
}


void DMA_free_prdt()
{
    // todo: remove prdt and prd.

    // free(IDE_DMA_physical_region);
}
