//
// Created by artypoole on 27/08/24.
//

#ifndef ATA_H
#define ATA_H
#include "types.h"

struct IDE_drive_t
{
    bool present;
    bool controller_id;
    bool drive_id;
    bool atapi_capable;
    bool dma_capable;
};

union ATA_status_t
{
    struct
    {
        u8 error : 1;
        u8 index : 1;
        u8 corrected : 1;
        u8 data_request : 1;
        u8 seek_complete : 1;
        u8 device_fault : 1;
        u8 ready : 1;
        u8 busy : 1;
    };

    u8 raw;
};


union ATA_error_t
{
    struct
    {
        u8 address_mark_not_found : 1;
        u8 track_zero_not_found : 1;
        u8 aborted_command : 1;
        u8 media_change_request : 1;
        u8 ID_not_found : 1;
        u8 media_changed : 1;
        u8 uncorrectable_data_error : 1;
        u8 bad_block_detected : 1;
    };

    u8 raw;
};

inline size_t sector_size = 2048;


IDE_drive_t find_drive();
int ATAPI_init(IDE_drive_t& drive);
int ATA_test_if_ATAPI();
u32 ATAPI_get_capacity();
int ATAPI_start_DMA_read(u16 n_sectors);
u8 get_ata_interrupt_reason();
ATA_status_t get_ata_device_status();
ATA_error_t get_ata_device_error();


#endif //ATA_H
