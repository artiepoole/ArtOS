//
// Created by artypoole on 27/08/24.
//

#ifndef IDE_H
#define IDE_H


#include "ATA.h"
#include "types.h"

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
int init_busmaster_device(u16 base_port, IDE_drive_t& drive);
u8* DMA_assign_prdt(u16 base_addr, u16 size);
void DMA_free_prdt(u16 base_addr);
void DMA_read(u8* dest, size_t n_bytes);
void IDE_handler(bool is_primary);
#endif //IDE_H
