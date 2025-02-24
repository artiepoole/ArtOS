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
// Created by artypoole on 22/09/24.
//

#ifndef ATADRIVE_H
#define ATADRIVE_H

#include "ATA_types.h"
#include "types.h"
#include "IDEDrive.h"


class ATADrive : public IDEDrive
{
public:
    explicit ATADrive(IDE_drive_info_t& drive_info);
    ~ATADrive() override;

    int populate_data() override;

    int start_DMA_read(u32 lba, size_t n_sectors) override;
    int seek(size_t LBA) override;
    int start_DMA_write(u32 lba, size_t n_sectors) override;
    int set_regs(const ATAPI_cmd_regs& regs) override;
    int populate_capabilities() override;
    u32 get_last_lba() override;


    int send_packet_PIO(const ATAPI_packet_t& packet) override;
    int send_packet_DMA(const ATAPI_packet_t& packet) override;
};

#endif //ATADRIVE_H
