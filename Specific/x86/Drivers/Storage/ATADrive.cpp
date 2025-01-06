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
