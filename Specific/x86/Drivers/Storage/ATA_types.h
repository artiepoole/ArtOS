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
// Created by artypoole on 09/09/24.
//

#ifndef ATA_TYPES_H
#define ATA_TYPES_H

#include "types.h"

struct IDE_drive_info_t
{
    bool present;
    bool controller_id;
    bool drive_id;
    bool packet_device;
    bool DMA_device;
    u16 base_port;
    u8 drive_data;
    u8 packet_size;
    u8 MW_DMA_modes;
    u8 UDMA_modes;
    u32 capacity_in_LBA;
    u16 sector_size;
    u16 block_size;
};

union ATA_status_t
{
    struct
    {
        u8 error : 1;
        u8 index : 1; // also sense data available bit.
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

union ATAPI_cmd_regs
{
    struct
    {
        u8 features;
        u8 sector_count;
        u8 LBA_low;
        u8 LBA_mid;
        u8 LBA_high;
    };

    struct
    {
        u8 bytes[5];
    };
};

struct ATAPI_inquiry_data_t
{
    u8 peripheral_type;
    u8 RMB;
    u8 version;
    u8 response_data_format;
    u8 addnl_length; ///< n - 4, Numbers of bytes following this one.
    u8 reserved_5;
    u8 reserved_6;
    u8 reserved_7;
    u8 vendor_info[8];
    u8 product_id[16];
    u8 product_revision_level[4];
};


union ATAPI_packet_t
{
    u8 bytes[12];
    u16 words[6];
};

struct ATAPI_ext_packet_t
{
    u8 opcode;
    u8 feature;
    u8 sector_count;
    u8 lba_low;
    u8 lba_mid;
    u8 lba_high;
    u8 drive_sel;
    u8 res0;
    u8 control;
    u8 res1[3];
    u8 additional[4];
};

#endif //ATA_TYPES_H
