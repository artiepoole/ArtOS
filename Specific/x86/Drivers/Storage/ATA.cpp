//
// Created by artypoole on 27/08/24.
//

#include "ATA.h"

#include <Files.h>
#include <IDE_handler.h>
#include <kernel.h>
#include <stdlib.h>

#include "Errors.h"
#include "logging.h"
#include "ports.h"


// TODO: consider enums for all of these Constants, in all files using them.
#define STATUS_ERR 0x01
#define STATUS_DRQ 0x08
#define STATUS_RDY 0x40
#define STATUS_BSY 0x80

// drive select
#define MAIN_DRIVE_SEL_CMD 0xE0 // LBA set and Dev not set. bits 5 and 7 also set.
#define ALT_DRIVE_SEL_CMD 0xF0  // LBA and Dev set

// ATA exclusuve command codes
#define ATA_IDENT_CMD 0xEC
#define READ_DMA_CMD_24 0xC8
#define READ_DMA_CMD_48 0x25
#define WRITE_DMA_CMD_24 0xCA
#define WRITE_DMA_CMD_48 0x35
#define SET_FEATURES_CMD 0xEF

// ATAPI exclusive commands
#define ATAPI_INQUIRY_CMD 0x12
#define ATAPI_GET_CAPACITY_CMD 0x25
#define ATAPI_IDENT_CMD 0xA1
#define PROCESS_PACKET_CMD 0xA0


// ATA and ATAPI commands
#define READ_SECTORS_CMD 0x20
#define DEV_CONFIG_IDENT_CMD 0xC2

// ATAPI packet opcodes
// #define ATAPI_READ_CD 0x80
// #define READ_10_CMD 0x28


// features subcommands:
#define ENABLE_WRITE_CACHE 0x02
#define DISABLE_WRITE_CACHE 0x82
#define ENABLE_READ_CACHE 0x10
#define DISABLE_READ_CACHE 0x55
#define TRANSFER_MODE 0x03

// DMA modes:
// Multiword
#define DMA_MW_0 0x00
#define DMA_MW_1 0x01
#define DMA_MW_2 0x02
// UDMA
#define UDMA_0 0x20
#define UDMA_1 0x21
#define UDMA_2 0x22
#define UDMA_3 0x23
#define UDMA_4 0x24
#define UDMA_5 0x25
#define UDMA_6 0x26

// ports
// primary: 0x1F0-0x1F7, 0x3F6
// secondary: 0x170-0x177, 0x376
#define DATA_OFFSET 0x0
#define ERROR_OFFSET 0x1 // read
#define FEATURES_OFFSET 0x1 // write
#define SECTOR_COUNT_OFFSET 0x2 // write
#define INTERRUPT_REASON_OFFSET 0x2 // read
#define LBA_LOW_OFFSET 0x3
#define LBA_MID_OFFSET 0x4
#define LBA_HIGH_OFFSET 0x5
#define DRIVE_SEL_OFFSET 0x6
#define STATUS_OFFSET 0x7
#define CMD_OFFSET 0x7
#define CONTROL_OFFSET 0x8

#define PRIMARY_BASE_PORT 0x1F0
#define SECONDARY_BASE_PORT 0x170

#define PRIMARY_ALT_STATUS_PORT 0x3F6
#define PRIMARY_CONTROL_PORT 0x3F6
#define PRIMARY_DRIVE_ADDR_PORT 0x3F7

#define SECONDARY_ALT_STATUS_PORT 0x376
#define SECONDARY_CONTROL_PORT 0x376
#define SECONDARY_DRIVE_ADDR_PORT 0x377

#define RESET_CONTROL 0x04


// Devide ID codes:
enum class ATA_DEVICE_TYPES
{
    DIRECT_ACCESS_DEVICE = 0x00,
    SEQUENTIAL_ACCESS_DEVICE = 0x01,
    PRINTER_DEVICE = 0x02,
    PROCESSOR_DEVICE = 0x03,
    WRITE_ONCE_DEVICE = 0x04,
    CD_ROM_DEVICE = 0x05,
};

/* scanner
 * optical memory
 * medium changer
 * comms dev
 * external_resources ACS IT8
 * external_resources ACT IT8
 * Array controller
 * enclosure services
 * reduced block command
 * optical card reader/writer
 * external_resources up to
 * 0x1F - unknown or unspecified
 */

IDE_drive_info_t* last_drive_info;

ATA_status_t ATA_get_status(IDE_drive_info_t* drive_info)
{
    if (last_drive_info != drive_info)
    {
        ATA_select_drive(drive_info);
        last_drive_info = drive_info;
    }
    ATA_status_t status{};
    status.raw = inb(drive_info->base_port + STATUS_OFFSET);
    return status;
}

ATA_status_t ATA_get_alt_status(IDE_drive_info_t* drive_info)
{
    if (last_drive_info != drive_info)
    {
        ATA_select_drive(drive_info);
        last_drive_info = drive_info;
    }
    ATA_status_t status{};
    if (drive_info->controller_id)
    {
        status.raw = inb(SECONDARY_ALT_STATUS_PORT);
    }
    else
    {
        status.raw = inb(PRIMARY_ALT_STATUS_PORT);
    }
    return status;
}


ATA_error_t ATA_get_error(IDE_drive_info_t* drive_info)
{
    if (last_drive_info != drive_info)
    {
        ATA_select_drive(drive_info);
        last_drive_info = drive_info;
    }
    ATA_error_t error{};
    error.raw = inb(drive_info->base_port + ERROR_OFFSET);
    return error;
}

u8 ATA_get_interrupt_reason(IDE_drive_info_t* drive_info)
{
    if (last_drive_info != drive_info)
    {
        ATA_select_drive(drive_info);
        last_drive_info = drive_info;
    }
    return inb(drive_info->base_port + INTERRUPT_REASON_OFFSET);
}

u16 ATA_get_n_bytes_to_read(IDE_drive_info_t* drive_info)
{
    if (last_drive_info != drive_info)
    {
        ATA_select_drive(drive_info);
        last_drive_info = drive_info;
    }
    return inb(drive_info->base_port + LBA_MID_OFFSET) | inb(drive_info->base_port + LBA_HIGH_OFFSET) << 8;
}

void ATA_select_drive(IDE_drive_info_t* drive_info)
{
    outb(drive_info->base_port + DRIVE_SEL_OFFSET, drive_info->drive_data);
    last_drive_info = drive_info;
    sleep_ns(500);
}

void ATA_reset_device(IDE_drive_info_t* drive_info)
{
    if (last_drive_info != drive_info)
    {
        ATA_select_drive(drive_info);
        last_drive_info = drive_info;
    }
    if (drive_info->controller_id)
    {
        outb(SECONDARY_CONTROL_PORT, RESET_CONTROL);
    }
    else
    {
        outb(PRIMARY_CONTROL_PORT, RESET_CONTROL);
    }
    sleep_ns(500); // wait after issuing reset then clear reset command.
    if (drive_info->controller_id)
    {
        outb(SECONDARY_CONTROL_PORT, 0x00);
    }
    else
    {
        outb(PRIMARY_CONTROL_PORT, 0x00);
    }
    ATA_poll_busy(drive_info); // wait for busy to be complete.
}

void ATA_poll_busy(IDE_drive_info_t* drive_info)
{
    if (last_drive_info != drive_info)
    {
        ATA_select_drive(drive_info);
        last_drive_info = drive_info;
    }
    ATA_status_t status = ATA_get_alt_status(drive_info);
    while (!status.error && status.busy)
    {
        status = ATA_get_alt_status(drive_info);
    }
}

void ATA_poll_until_DRQ(IDE_drive_info_t* drive_info)
{
    if (last_drive_info != drive_info)
    {
        ATA_select_drive(drive_info);
        last_drive_info = drive_info;
    }
    ATA_status_t status = ATA_get_alt_status(drive_info);
    while (!status.data_request && !status.error)
    {
        status = ATA_get_alt_status(drive_info);
    }
}

int ATAPI_ident(IDE_drive_info_t* drive_info, u16* identity_data)
{
    ATA_select_drive(drive_info);
    LOG("Sending ATAPI IDENTIFY command.");
    outb(drive_info->base_port + CMD_OFFSET, ATAPI_IDENT_CMD);
    ATA_poll_busy(drive_info);
    ATA_poll_until_DRQ(drive_info);
    //todo: test whether you can skip reading all we don't use
    if (ATA_status_t status = ATA_get_alt_status(drive_info); status.data_request && !status.error) // data request is true and error is false
    {
        for (size_t i = 0; i < 256; i++)
        {
            identity_data[i] = inw(drive_info->base_port + DATA_OFFSET);
        }
        status = ATA_get_alt_status(drive_info);
        if (status.data_request) { LOG("Read 256 words but drq still set?"); };
        if (identity_data[0] >> 14 == 0x2)
        {
            LOG("ATAPI device confirmed.");
            drive_info->packet_device = true;
            switch (identity_data[0] & 0b11)
            {
            case 0:
                {
                    LOG("ATAPI packet size: 12 bytes");
                    drive_info->packet_size = 12;
                    break;
                }
            case 1:
                {
                    LOG("ATAPI packet size: 16 bytes");
                    drive_info->packet_size = 16;
                    break;
                }
            default:
                {
                    LOG("Unexpected ATAPI packet size.");
                    break;
                }
            }
            switch ((identity_data[0] & 0x1F00) >> 8)
            {
            case static_cast<u32>(ATA_DEVICE_TYPES::CD_ROM_DEVICE):
                {
                    LOG("Identified CD ROM device.");
                    drive_info->block_size = 2048;
                    drive_info->sector_size = 2048;
                    break;
                }
            default:
                {
                    LOG("Unhandled device class. Using default sector and block sizes.");
                    drive_info->block_size = 512;
                    drive_info->sector_size = 512;
                    break;
                }
            }
        }
        else
        {
            LOG("Device not ATAPI?");
            return -DEVICE_ERROR;
        }
        // serial number: words 10-19
        // firmware number: 23-26
        // model nmber: 27-46
        // 53 specifies whether other fields are valid or not including UDMA data
        // 80 says maximum ATAPI revision support

        if (identity_data[63] != 0)
        {
            LOG("MW DMA modes detected.");
            drive_info->MW_DMA_modes = identity_data[63] & 0xFF ;
            drive_info->DMA_device = true;
        }
        if (identity_data[88] != 0)
        {
            LOG("UDMA modes detected.");
            drive_info->UDMA_modes = identity_data[88] & 0xFF;
            drive_info->DMA_device = true;
        }
        return 0;
    }
    return -DEVICE_ERROR;
}

int ATA_ident(IDE_drive_info_t* drive_info, [[maybe_unused]] u16* identity_data)
{
    ATA_select_drive(drive_info);
    // TODO: Not implemented yet
    return -NOT_IMPLEMENTED;
}

int ATA_is_packet_device(IDE_drive_info_t* drive_info)
{
    ATA_reset_device(drive_info);

    u8 signatures[4] = {0};
    signatures[0] = inb(drive_info->base_port + SECTOR_COUNT_OFFSET);
    signatures[1] = inb(drive_info->base_port + LBA_LOW_OFFSET);
    signatures[2] = inb(drive_info->base_port + LBA_MID_OFFSET);
    signatures[3] = inb(drive_info->base_port + LBA_HIGH_OFFSET);

    if ((!signatures[0]) == 0x1 || (!signatures[1]) == 0x1)
    {
        LOG("Drive signature not present");
        return -DEVICE_ERROR;
    }
    if (signatures[2] == 0 && signatures[3] == 0)
    {
        LOG("Drive is ATA device, no packet support.");
        return 0;
    }
    if (signatures[2] == 0x14 && signatures[3] == 0xEB)
    {
        LOG("Drive is ATAPI device. Packets supported.");
        drive_info->packet_device = true;
        return 1;
    }
    LOG("Unexpected device signature.");
    return -DEVICE_ERROR;
}

int ATAPI_set_max_dma_mode(const bool supports_udma, IDE_drive_info_t* drive_info)
{
    ATA_select_drive(drive_info);
    u8 dma_mode = 0;

    if (supports_udma)
    {
        while (drive_info->UDMA_modes & 0x1 << dma_mode) { dma_mode++; }
        dma_mode += 0x20;
    }
    else
    {
        while (drive_info->MW_DMA_modes & 0x1 << dma_mode) { dma_mode++; }
    }
    LOG("Setting DMA mode: ", dma_mode);

    outb(drive_info->base_port + FEATURES_OFFSET, TRANSFER_MODE); // Subcommand to set transfer mode
    outb(drive_info->base_port + SECTOR_COUNT_OFFSET, dma_mode); // Set DMA mode 1 (or use 0x20, 0xC5, etc. for other modes)
    outb(drive_info->base_port + CMD_OFFSET, SET_FEATURES_CMD); // Send the Set Features command

    if (ATA_status_t status = ATA_get_alt_status(drive_info); status.error)
    {
        [[maybe_unused]]ATA_error_t error = ATA_get_error(drive_info);
        LOG("Error enabling DMA. Raw error", error.raw);
        return -DEVICE_ERROR;
    }

    // todo: Test if the mode was set using the identify command?
    return dma_mode;
}


/* Finds all drives
 */
// Must be passed a list of IDE_drive_info_t[4].
int populate_drives_list(IDE_drive_info_t* drive_list)
{
    int n_found_drives = 0;
    constexpr bool are_secondary_controllers[4] = {false, false, true, true};
    constexpr u16 base_ports[4] = {PRIMARY_BASE_PORT,PRIMARY_BASE_PORT, SECONDARY_BASE_PORT, SECONDARY_BASE_PORT};

    constexpr bool are_secondary_drives[4] = {false, true, false, true};
    constexpr u8 drive_sel_vals[4] = {MAIN_DRIVE_SEL_CMD, ALT_DRIVE_SEL_CMD, MAIN_DRIVE_SEL_CMD, ALT_DRIVE_SEL_CMD};

    // Find all connected drives
    for (int i = 0; i < 4; i++)
    {
        // IDE_drive_info_t* drive =  &drives[i];
        IDE_drive_info_t drive_info{};
        drive_info.base_port = base_ports[i];
        drive_info.drive_data = drive_sel_vals[i];
        drive_info.drive_id = are_secondary_drives[i];
        drive_info.controller_id = are_secondary_controllers[i];

        // select drive
        ATA_select_drive(&drive_info);

        ATA_status_t status{};
        status.raw = inb(drive_info.base_port + STATUS_OFFSET);
        drive_info.present = false;
        if (status.ready)
        {
            drive_info.present = true;
            n_found_drives++;
            LOG("Device detected. IDE", static_cast<int>(drive_info.controller_id), " drive", static_cast<int>(drive_info.drive_id));
            drive_info.packet_device = static_cast<bool>(ATA_is_packet_device(&drive_info));
            drive_list[n_found_drives] = drive_info;
            continue;
        }
        if (status.error)
        {
            LOG("Device ERROR detected. IDE", drive_info.controller_id, " drive", drive_info.drive_id);
            drive_info.present = false;
        }
    }
    return n_found_drives;
}
