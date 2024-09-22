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

// Have only made it possible to have one PRD for each device. Therefore End of Table is always 1.
u8 * DMA_init_PRDT(u16 base_port);
u8* DMA_assign_prdt(u16 base_addr, u16 size);
void DMA_free_prdt(u16 base_addr);
void DMA_read(u8* dest, size_t n_bytes);
// void IDE_handler(bool is_primary);


#endif //IDE_H
