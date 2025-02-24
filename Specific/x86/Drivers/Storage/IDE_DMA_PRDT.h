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
// Created by artypoole on 27/08/24.
//

#ifndef IDE_H
#define IDE_H

#include "types.h"


#define PRIMARY_CMD_OFFSET 0x00
#define PRIMARY_STATUS_OFFSET 0x02
#define PRIMARY_PRDT_START_OFFSET  0x04

#define SECONDARY_CMD_OFFSET 0x08
#define SECONDARY_STATUS_OFFSET 0x0A
#define SECONDARY_PRDT_START_OFFSET 0x0C

//command settings

#define DEV_TO_MEM 0x01
#define MEM_TO_DEV 0x00
#define DMA_MODE 0x08


// Physical Region Descriptor Table
struct PRD_t
{
    union
    {
        struct
        {
            u64 base_addr : 32;
            u64 length_in_b : 16;
            u64 pad : 15;
            u64 end_of_table : 1;
        };

        u64 raw;
    };
};

struct PRDT_t
{
    PRD_t descriptor;

}__attribute__((aligned(4)));


struct BM_cmd_t
{
    union
    {
        struct
        {
            u8 start_stop : 1;
            u8 res_0 : 2;
            u8 rw_ctrl : 1;
            u8 res_1 : 4;
        };

        u8 raw;
    };
};

struct BM_status_t
{
    union
    {
        struct
        {
            u8 IDE_active : 1;
            u8 error : 1;
            u8 interrupt : 1;
            u8 res_0 : 2;
            u8 drive_0_dma_capable : 1;
            u8 drive_1_dma_capable : 1;
            u8 simplex_only : 1;
        };

        u8 raw;
    };
};

// TODO: create a PRD object and make it so that there can be 2 PRDTs (one for each DMA controller)
// Have only made it possible to have one PRD for each device. Therefore End of Table is always 1.
u8* DMA_init_PRDT(bool controller_id, u16 base_port);
void DMA_free_prdt(u16 base_addr);


#endif //IDE_H
