//
// Created by artypoole on 27/08/24.
//
#include "logging.h"
#include "BusMasterController.h"
#include "IDE_DMA_PRDT.h"


#include <ports.h>

#define STATUS_OFFSET PRIMARY_STATUS_OFFSET
#define CMD_OFFSET PRIMARY_CMD_OFFSET
#define PRD_OFFSET PRIMARY_PRD_OFFSET

u32 prd_size = 65536;

BusMasterController::BusMasterController(u16 new_base_port, IDE_drive_info_t* drive)
{

    // u16 command = PCI_IDE_controller->set_command_bit(2, true);
    //
    // if ((command & 0x1 << 2) > 0)
    // {
    //     LOG("IDE busmastering enabled");
    // }
    // else
    // {
    //     LOG("IDE busmastering not enabled. Aborting");
    //     return;
    // }



    if (drive->controller_id)
    {
        base_port = new_base_port + 0x08;
    }
    else
    {
        base_port = new_base_port;
    }

    LOG("Initialising busmaster device.");
    BM_status_t status = get_status();
    status.error = 1;
    status.interrupt = 1;
    if (drive->DMA_device)
    {
        if (drive->drive_id) { status.drive_1_dma_capable = 1; }
        else { status.drive_0_dma_capable = 1; }
    }
    status = set_status(status);
    if (status.error)
    {
     // TODO: handle init errors
    }
    physical_region = DMA_init_PRDT(base_port);

}

BM_cmd_t BusMasterController::set_cmd(const BM_cmd_t cmd) const
{
    outb(base_port + CMD_OFFSET, cmd.raw);
    return get_cmd();
}

BM_cmd_t BusMasterController::get_cmd() const
{
    BM_cmd_t cmd{};
    cmd.raw = inb(base_port + CMD_OFFSET);
    return cmd;
}


BM_status_t BusMasterController::set_status(const BM_status_t status) const
{
    outb(base_port + STATUS_OFFSET, status.raw);
    return get_status();
}

BM_status_t BusMasterController::get_status() const
{
    BM_status_t status{};
    status.raw = inb(base_port + STATUS_OFFSET);
    return status;
}
