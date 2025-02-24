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

#include "ATADrive.h"
#include "Errors.h"


ATADrive::ATADrive(IDE_drive_info_t& drive_info)
{
    this->drive_info = &drive_info;
}

ATADrive::~ATADrive() = default;

int ATADrive::populate_data()
{
    return -NOT_IMPLEMENTED;
}


int ATADrive::start_DMA_read(u32 lba, size_t n_sectors)
{
    return -NOT_IMPLEMENTED;
}

int ATADrive::seek(size_t LBA)
{
    return -NOT_IMPLEMENTED;
}

int ATADrive::start_DMA_write(u32 lba, size_t n_sectors)
{
    return -NOT_IMPLEMENTED;
}

int ATADrive::set_regs(const ATAPI_cmd_regs& regs)
{
    return -NOT_IMPLEMENTED;
}

int ATADrive::populate_capabilities()
{
    return -NOT_IMPLEMENTED;
}

u32 ATADrive::get_last_lba()
{
    return -NOT_IMPLEMENTED;
}

int ATADrive::send_packet_PIO(const ATAPI_packet_t& packet)
{
    return -NOT_IMPLEMENTED;
}

int ATADrive::send_packet_DMA(const ATAPI_packet_t& packet)
{
    return -NOT_IMPLEMENTED;
}
