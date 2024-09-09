//
// Created by artypoole on 27/08/24.
//

#ifndef ATA_H
#define ATA_H
#include "ATAPIDrive.h"
#include "ATA_types.h"
#include "types.h"


inline size_t sector_size = 2048;

// Must be passed a list of IDE_drive_t[4].
int populate_drives_list(ATAPIDrive*& atapi_drives);

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

int ATAPI_set_max_dma_mode(bool udma, IDE_drive_info_t* drive_info);

void ATAPI_send_packet_DMA(IDE_drive_info_t* drive_info, ATAPI_packet_t& packet);


#endif //ATA_H
