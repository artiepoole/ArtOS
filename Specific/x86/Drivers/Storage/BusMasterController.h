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

#ifndef BUSMASTER_H
#define BUSMASTER_H
#include "IDE_DMA_PRDT.h"
#include "ATA_types.h"

// I am only supporting I/O port busmaster devices here.
class BusMasterController
{
public:
    BusMasterController(u16 new_base_port, IDE_drive_info_t* drive);
    void start_transfer();
    void stop_transfer();
    void prepare_dma_transfer();
    BM_status_t get_status() const;
    BM_status_t set_status(BM_status_t status) const;
    BM_cmd_t get_cmd() const;
    BM_cmd_t set_cmd(BM_cmd_t cmd) const;
    u8* physical_region;
    u16 base_port;

};

// LOG("Initialising busmaster device.");
// bus_master_base_port = new_base_port;
// is_primary_IDE = !drive->controller_id;
// is_main_IDE = !drive->drive_id;
//
// BM_status_t status = get_busmaster_status();
// status.error = 1;
// status.interrupt = 1; //  writing 1 clears the bit
// status.drive_0_dma_capable = 1;
// status = set_busmaster_status(status);


#endif //BUSMASTER_H
