//
// Created by artypoole on 05/09/24.
//

#ifndef ATAPIDRIVE_H
#define ATAPIDRIVE_H

#include "ATA_types.h"
#include "types.h"
#include "IDEDrive.h"


class ATAPIDrive : public IDEDrive
{
public:
    explicit ATAPIDrive(IDE_drive_info_t& drive_info);
    ~ATAPIDrive() override;

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


#endif //ATAPIDRIVE_H
