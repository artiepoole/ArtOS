//
// Created by artypoole on 05/09/24.
//

#include "IDEStorageContainer.h"

#include <ATAPIDrive.h>
#include <Errors.h>
#include <IDE_DMA_PRDT.h>
#include <string.h>

#include "IDE_handler.h"
#include "logging.h"





IDEStorageContainer::IDEStorageContainer(ATAPIDrive* drive, PCIDevice* pci_dev, BusMasterController* bm_dev)
{
    this->drive_dev = drive;
    this->pci_dev = pci_dev;
    this->bm_dev = bm_dev;
    // Set PCI cmd bit
    // set BM bit
    // clear BM status
    if (!(pci_dev->get_command() & 0x4))
    {
        pci_dev->set_command_bit(2, true); // enable busmastering
    }
    BM_status_t bm_status = bm_dev->get_status();
    bm_status.error = 1;
    bm_status.interrupt = 1; //  writing 1 clears the bit

    IDE_add_device(this); // add interrupt handling
    IDE_remove_device(this->drive_dev); // Stop interrupts from going directly to the contained device.
}

// IDEStorageContainer::~IDEStorageContainer()
// {
//     IDE_remove_device(this);
//     // TODO: remove PRDT?
// }

int IDEStorageContainer::read(u8* dest, u32 n_bytes)
{
    if (n_bytes == 0) return -1;
    u16 n_sectors = (n_bytes + (sector_size - 1)) / sector_size; // round up division

    int ret_val = 0;
    ret_val = prep_DMA_read(n_sectors); // should set up ATA stuff and then set up BM stuff
    if(ret_val!=0){return ret_val;}
    start_DMA_transfer(); // should just set BM start_stop
    ret_val = wait_for_DMA_transfer(); // should poll/wait/check status of each device.
    if(ret_val!=0){return ret_val;}
    ret_val = stop_DMA_read(); // should just reset BM start_stop
    if(ret_val!=0){return ret_val;}
    memcpy(dest, bm_dev->physical_region, n_bytes);
    return 0;
}

void IDEStorageContainer::notify()
{
    // Should just check if this controller/drive_dev was to be handled or not.
    if (!(BM_waiting_for_transfer | drive_dev->waiting_for_transfer)) return;

    BM_status_t bm_status = bm_dev->get_status();
    ATA_status_t ata_status = drive_dev->get_status();
    if (ata_status.error)
    {
        LOG("ATA device errored");
        drive_dev->waiting_for_transfer = false;
    }

    if (ata_status.data_request)
    {
        LOG("ATA sent command");
        drive_dev->waiting_for_transfer = false;
    }
    else
    {
        LOG("ATA probably didn't send command");
        BM_waiting_for_transfer = false;
    }

    if (bm_status.interrupt)
    {
        LOG("BM interrupt raised.");
        bm_status.interrupt = true;
        bm_status = bm_dev->set_status(bm_status);
    }

}

int IDEStorageContainer::prep_DMA_read(size_t n_sectors)
{
    // Drive_waiting_for_transfer = true;
    drive_dev->start_DMA_read(n_sectors);
    // while (Drive_waiting_for_transfer){};
    auto cmd = bm_dev->get_cmd();
    cmd.rw_ctrl = DEV_TO_MEM;
    cmd = bm_dev->set_cmd(cmd); // apparently you set this then enable in separate writes.
    return 0;
}

void IDEStorageContainer::start_DMA_transfer()
{
    BM_waiting_for_transfer = true;
    BM_cmd_t bm_cmd = bm_dev->get_cmd();
    bm_cmd.start_stop = 1;
    bm_dev->set_cmd(bm_cmd);
}

int IDEStorageContainer::stop_DMA_read()
{
    BM_cmd_t bm_cmd = bm_dev->get_cmd();
    bm_cmd.start_stop = 0;
    bm_dev->set_cmd(bm_cmd);
    if (bm_dev->get_status().error)
    {
        return -DEVICE_ERROR;
    }
    return 0;
}

int IDEStorageContainer::wait_for_DMA_transfer() const
{
    // TODO: This is not working properly when running full speed.
    BM_status_t bm_status = bm_dev->get_status();
    while (BM_waiting_for_transfer & bm_status.IDE_active & !bm_status.error)
    {
        bm_status = bm_dev->get_status();
    }
    if (BM_waiting_for_transfer)
    {
        LOG("BM transfer complete didn't send interrupt?");
        return -DEVICE_ERROR;
    }
    return 0;
}



