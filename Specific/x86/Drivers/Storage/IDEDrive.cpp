//
// Created by artypoole on 22/09/24.
//

#include "IDEDrive.h"
#include "ATA.h"
#include "logging.h"

IDEDrive::~IDEDrive() = default;

void IDEDrive::notify()
{
    // TODO: Expand this to handle all the interrupt types by adding "waiting for" flags for ATA drives.
    if (!waiting_for_transfer) { return; }
    // must be read to ackknowledge interrupt
    [[maybe_unused]] ATA_status_t ata_status = get_status();
    [[maybe_unused]] u8 ata_interrupt_reason = get_interrupt_reason();
#ifndef NDEBUG
    switch (ata_interrupt_reason)
    {
    case 0:
        {
            LOG("No IDE interrupt reason set");
            break;
        }
    case 1:
        {
            LOG("Command/Data bit set");
            break;
        }
    case 2:
        {
            LOG("IO bit set");
            break;
        }
    case 3:
        {
            LOG(" Data and IO bits set");
            break;
        }
    default:
        {
            LOG("interrupt reason: ", ata_interrupt_reason);
            break;
        }
    }

    if (ata_status.error)
    {
        LOG("ATA device errored");
    }

    if (ata_status.data_request)
    {
        LOG("ATA is waiting for data transfer.");
    }
#endif
    waiting_for_transfer = false;
}


ATA_status_t IDEDrive::get_status() const
{
    return ATA_get_status(drive_info);
}

ATA_status_t IDEDrive::get_alt_status() const
{
    return ATA_get_alt_status(drive_info);
}

ATA_error_t IDEDrive::get_error() const
{
    return ATA_get_error(drive_info);
}

u8 IDEDrive::get_interrupt_reason() const
{
    return ATA_get_interrupt_reason(drive_info);
}
