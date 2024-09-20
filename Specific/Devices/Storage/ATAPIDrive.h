//
// Created by artypoole on 05/09/24.
//

#ifndef ATADRIVE_H
#define ATADRIVE_H

#include "ATA_types.h"
#include "types.h"
#include "IDE_notifiable.h"


class ATAPIDrive : public IDE_notifiable
{
public:
    void notify() override;


    ATAPIDrive() = default;
    ~ATAPIDrive() override = default;
    int populate_data(IDE_drive_info_t* drive);

    int start_DMA_read(u32 lba, size_t n_sectors);
    int seek(size_t LBA);
    int start_DMA_write(u32 lba, size_t n_sectors);
    [[nodiscard]] ATA_status_t get_status();
    [[nodiscard]] ATA_status_t get_alt_status();
    [[nodiscard]] ATA_error_t get_error();
    [[nodiscard]] u8 get_interrupt_reason();
    int set_regs(const ATAPI_cmd_regs& regs);
    int populate_capabilities();
    u32 get_last_lba();


    int send_packet_PIO(const ATAPI_packet_t& packet);
    int send_packet_DMA(const ATAPI_packet_t& packet);


    IDE_drive_info_t* drive_info;

    volatile bool waiting_for_transfer = false;
    u16 identity_data[256]{0};
};


#endif //ATADRIVE_H
