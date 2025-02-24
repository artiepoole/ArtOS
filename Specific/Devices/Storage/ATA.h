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

#ifndef ATA_H
#define ATA_H
#include "ATA_types.h"
#include "types.h"


// Must be passed a list of IDE_drive_t[4].
int populate_drives_list(IDE_drive_info_t* drive_list);

ATA_status_t ATA_get_status(IDE_drive_info_t* drive_info);

ATA_status_t ATA_get_alt_status(IDE_drive_info_t* drive_info);

ATA_error_t ATA_get_error(IDE_drive_info_t* drive_info);

u8 ATA_get_interrupt_reason(IDE_drive_info_t* drive_info);

u16 ATA_get_n_bytes_to_read(IDE_drive_info_t* drive_info);

void ATA_select_drive(IDE_drive_info_t* drive_info);

void ATA_reset_device(IDE_drive_info_t* drive_info);

void ATA_poll_busy(IDE_drive_info_t* drive_info);

void ATA_poll_until_DRQ(IDE_drive_info_t* drive_info);

int ATAPI_ident(IDE_drive_info_t* drive_info, u16* identity_data);

int ATA_ident(IDE_drive_info_t* drive_info, u16* identity_data);

int ATA_is_packet_device(IDE_drive_info_t* drive_info);

int ATAPI_set_max_dma_mode(bool supports_udma, IDE_drive_info_t* drive_info);

void ATAPI_send_packet_DMA(IDE_drive_info_t* drive_info, ATAPI_packet_t& packet);


#endif //ATA_H
