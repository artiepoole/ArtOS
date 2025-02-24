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
// Created by artypoole on 13/09/24.
//

#ifndef ISO_FS_H
#define ISO_FS_H
#include "types.h"

struct iso_fs_datetime
{
    u8 years_since_1900;
    u8 month;
    u8 monthday;
    u8 hour;
    u8 minute;
    u8 second;
    i8 gmt_offset_in_quarter_hours;
}__attribute__((packed));

struct iso_directory_record_header
{
    u8 record_length;
    u8 ext_length;
    u32 extent_loc_LE;
    u32 extent_loc_BE;
    u32 data_length_LE; // bytes
    u32 data_length_BE; // bytes
    iso_fs_datetime datetime;
    u8 flags;
    u8 unit_size;
    u8 interleave_size;
    u16 vol_sequence_number_LE;
    u16 vol_sequence_number_BE;
    u8 file_name_length;
}__attribute__((packed));

struct iso_directory_record
{
    iso_directory_record_header header;
    char* filename = nullptr;
    char* system_use = nullptr;
};

struct iso_path_table_entry_header
{
    u8 name_length;
    u8 extended_info_length;
    u32 extent_loc;
    u16 parent_dir_id;
}__attribute__((packed));

struct iso_path_table_entry
{
    iso_path_table_entry_header header;
    char* dir_name = nullptr;
    iso_directory_record* files = nullptr;
};


struct iso_primary_volume_descriptor_t
{
    u8 descriptor_type;
    char identifier[5];
    u8 version;
    u8 res0;
    char system_id[32];
    char volume_id[32];
    u8 res1[8];
    u32 volume_space_size_LE;
    u32 volume_space_size_BE;
    u8 res2[32];
    u16 volume_set_size_LE;
    u16 volume_set_size_BE;
    u16 volume_sequence_number_LE;
    u16 volume_sequence_number_BE;
    u16 logical_block_size_LE; // bytes
    u16 logical_block_size_BE; // bytes
    u32 path_table_size_LE; // bytes
    u32 path_table_size_BE; // bytes
    u32 path_l_table_loc_lba; // LBA = 2048*bytes
    u32 path_l_optional_loc_lba; // LBA = 2048*bytes
    u32 path_m_table_loc_lba; // LBA = 2048*bytes
    u32 path_m_optional_loc_lba; // LBA = 2048*bytes
    iso_directory_record_header root_dir_header;
    char root_dir_name[1];
    char volume_set_identifier[128];
    char publisher_identifier[128];
    char data_preparer_identifier[128];
    char application_identifier[128];
    char copyright_file_id[37];
    char abstract_file_id[37];
    char bibliographic_file_id[37];
    char creation_datetime[17];
    char modification_datetime[17];
    char expiration_datetime[17];
    char effective_datetime[17];
    u8 file_structure_version;
    u8 res3;
    u8 application_specific[512];
    u8 res4[653];
};

struct file_id_ext_header
{
    char tag[2];
    u8 len;
};

#endif //ISO_FS_H
