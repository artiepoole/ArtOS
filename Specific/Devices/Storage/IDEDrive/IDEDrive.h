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

#ifndef IDEDRIVE_H
#define IDEDRIVE_H


#include "IDE_notifiable.h"
#include "ATA_types.h"
#include "types.h"

class IDEDrive : public IDE_notifiable
{
public:
    ~IDEDrive() override;

    void notify() override;

    [[nodiscard]] ATA_status_t get_status() const;
    [[nodiscard]] ATA_status_t get_alt_status() const;
    [[nodiscard]] ATA_error_t get_error() const;
    [[nodiscard]] u8 get_interrupt_reason() const;

    virtual int populate_data() =0;

    virtual int start_DMA_read(u32 lba, size_t n_sectors) =0;
    virtual int seek(size_t LBA) =0;
    virtual int start_DMA_write(u32 lba, size_t n_sectors) =0;

    virtual int set_regs(const ATAPI_cmd_regs& regs) =0;
    virtual int populate_capabilities() =0;
    virtual u32 get_last_lba() =0;

    virtual int send_packet_PIO(const ATAPI_packet_t& packet) =0;
    virtual int send_packet_DMA(const ATAPI_packet_t& packet) =0;

    IDE_drive_info_t* get_drive_info() const { return drive_info; }
    [[nodiscard]] bool is_waiting_for_transfer() const { return waiting_for_transfer; }
    void set_waiting_for_transfer(const bool state) { waiting_for_transfer = state; }

protected:
    IDE_drive_info_t* drive_info = nullptr;
    volatile bool waiting_for_transfer = false;
    u16 identity_data[256]{0};
};

#endif //IDEDRIVE_H
