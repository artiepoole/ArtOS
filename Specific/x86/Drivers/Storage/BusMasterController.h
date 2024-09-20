//
// Created by artypoole on 27/08/24.
//

#ifndef BUSMASTER_H
#define BUSMASTER_H
#include "IDE_DMA_PRDT.h"

#include "ATA.h"

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
