//
// Created by artypoole on 27/08/24.
//

#include "ATA.h"

#include <APIC.h>
#include <IDE_handler.h>
#include <kernel.h>
#include <stdlib.h>

#include "Errors.h"
#include "IDT.h"
#include "logging.h"
#include "PIT.h"
#include "ports.h"

// ATA ports
// https://wiki.osdev.org/ATA_PIO_Mode
// https://wiki.osdev.org/ATAPI


// ATA commands
// https://wiki.osdev.org/ATAPI
// https://wiki.osdev.org/ATA_Command_Matrix


// ATAPI reference:
// http://web.archive.org/web/20221119212108/https://node1.123dok.com/dt01pdf/123dok_us/001/139/1139315.pdf.pdf?X-Amz-Content-Sha256=UNSIGNED-PAYLOAD&X-Amz-Algorithm=AWS4-HMAC-SHA256&X-Amz-Credential=7PKKQ3DUV8RG19BL%2F20221119%2F%2Fs3%2Faws4_request&X-Amz-Date=20221119T211954Z&X-Amz-SignedHeaders=host&X-Amz-Expires=600&X-Amz-Signature=c4154afd7a42db51d05db14370b8e885c3354c14cb66986bffcfd1a9a0869ee2


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
    // sleep(1);
    u64 start = get_tick_ns();
    while (get_tick_ns() - start < 500){}// wait at least 500ns

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
    sleep(10);
    if (drive_info->controller_id)
    {
        outb(SECONDARY_CONTROL_PORT, 0x00);
    }
    else
    {
        outb(PRIMARY_CONTROL_PORT, 0x00);
    }
}

void ATA_poll_busy(IDE_drive_info_t* drive_info)
{
    if (last_drive_info != drive_info)
    {
        ATA_select_drive(drive_info);
        last_drive_info = drive_info;
    }
    ATA_status_t status = ATA_get_alt_status(drive_info);
    while (status.busy & !status.error)
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
    while (!status.data_request & !status.error)
    {
        status = ATA_get_alt_status(drive_info);
    }
}

int ATAPI_ident(IDE_drive_info_t* drive, u16* identity_data)
{
    ATA_select_drive(drive);

    outb(drive->base_port + CMD_OFFSET, ATAPI_IDENT_CMD);
    ATA_poll_busy(drive);
    ATA_poll_until_DRQ(drive);
    //todo: test whether you can skip reading all we don't use
    if (ATA_status_t status = ATA_get_alt_status(drive); status.data_request and !status.error) // data request is true and error is false
    {
        for (size_t i = 0; i < 256; i++)
        {
            identity_data[i] = inw(drive->base_port + DATA_OFFSET);
        }
        status = ATA_get_alt_status(drive);
        if (status.data_request) { LOG("Read 256 words but drq still set?"); };
        if (identity_data[0] >> 14 == 0x2)
        {
            LOG("ATAPI device confirmed.");
            drive->packet_device = true;
            switch (identity_data[0] & 0b11)
            {
            case 0:
                {
                    LOG("ATAPI packet size: 12 bytes");
                    drive->packet_size = 12;
                    break;
                }
            case 1:
                {
                    LOG("ATAPI packet size: 16 bytes");
                    drive->packet_size = 16;
                    break;
                }
            default:
                {
                    LOG("Unexpected ATAPI packet size.");
                    break;
                }
            }
        }
        else
        {
            LOG("Device not ATAPI?");
            return -DEVICE_ERROR;
        }
        if (identity_data[63] != 0)
        {
            LOG("MW DMA modes detected.");
            drive->MW_DMA_modes = identity_data[63];
            drive->DMA_device = true;
        }
        if (identity_data[88] != 0)
        {
            LOG("UDMA modes detected.");
            drive->UDMA_modes = identity_data[88];
            drive->DMA_device = true;
        }
        return 0;
    }
    return -DEVICE_ERROR;
}

int ATA_ident(IDE_drive_info_t* drive, u16* identity_data)
{
    ATA_select_drive(drive);
    // TODO: Not implemented yet
    return -NOT_IMPLEMENTED;
}

int ATA_is_packet_device(IDE_drive_info_t* drive)
{
    ATA_reset_device(drive);


    u8 signatures[4] = {0};
    signatures[0] = inb(drive->base_port + SECTOR_COUNT_OFFSET);
    signatures[1] = inb(drive->base_port + LBA_LOW_OFFSET);
    signatures[2] = inb(drive->base_port + LBA_MID_OFFSET);
    signatures[3] = inb(drive->base_port + LBA_HIGH_OFFSET);

    if (!signatures[0] == 0x1 | !signatures[1] == 0x1)
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
        drive->packet_device = true;
        return 1;
    }
    LOG("Unexpcected device signature.");
    return -DEVICE_ERROR;
}

int ATAPI_set_max_dma_mode(const bool udma, IDE_drive_info_t* drive)
{
    ATA_select_drive(drive);
    u8 dma_mode = 0;

    if (udma)
    {
        while (drive->UDMA_modes & 0x1 << dma_mode) { dma_mode++; }
        dma_mode += 0x20;
    }
    else
    {
        while (drive->MW_DMA_modes & 0x1 << dma_mode) { dma_mode++; }
    }
    LOG("Setting DMA mode: ", dma_mode);

    outb(drive->base_port + FEATURES_OFFSET, TRANSFER_MODE); // Subcommand to set transfer mode
    outb(drive->base_port + SECTOR_COUNT_OFFSET, dma_mode); // Set DMA mode 1 (or use 0x20, 0xC5, etc. for other modes)
    outb(drive->base_port + CMD_OFFSET, SET_FEATURES_CMD); // Send the Set Features command

    if (ATA_status_t status = ATA_get_alt_status(drive); status.error)
    {
        ATA_error_t error = ATA_get_error(drive);
        LOG("Error enabling DMA. Raw error", error.raw);
        return -DEVICE_ERROR;
    }

    // todo: Test if the mode was set using the identify command?
    return dma_mode;
}


/* Finds all drives
 */
// Must be passed a list of IDE_drive_info_t[4].
int populate_drives_list(ATAPIDrive*& atapi_drives)
{
    IDE_drive_info_t* found_drives[4] = {};
    int n_found_drives = 0;
    constexpr bool are_secondary_controllers[4] = {false, false, true, true};
    constexpr u16 base_ports[4] = {PRIMARY_BASE_PORT,PRIMARY_BASE_PORT, SECONDARY_BASE_PORT, SECONDARY_BASE_PORT};

    constexpr bool are_secondary_drives[4] = {false, true, false, true};
    constexpr u8 drive_sel_vals[4] = {MAIN_DRIVE_SEL_CMD, ALT_DRIVE_SEL_CMD, MAIN_DRIVE_SEL_CMD, ALT_DRIVE_SEL_CMD};

    // Find all connected drives
    for (int i = 0; i < 4; i++)
    {
        // IDE_drive_info_t* drive =  &drives[i];
        auto drive_info = new IDE_drive_info_t();
        drive_info->base_port = base_ports[i];
        drive_info->drive_data = drive_sel_vals[i];
        drive_info->drive_id = are_secondary_drives[i];
        drive_info->controller_id = are_secondary_controllers[i];

        // select drive
        ATA_select_drive(drive_info);

        ATA_status_t status{};
        status.raw = inb(drive_info->base_port + STATUS_OFFSET);

        if (status.ready)
        {
            drive_info->present = true;
            found_drives[n_found_drives] = drive_info;
            n_found_drives++;
            LOG("Device detected. IDE", static_cast<int>(drive_info->controller_id), " drive", static_cast<int>(drive_info->drive_id));
            continue;
        }
        if (status.error)
        {
            LOG("Device ERROR detected. IDE", drive_info->controller_id, " drive", drive_info->drive_id);
            drive_info->present = false;
        }
        drive_info->present = false;
        free(drive_info);
    }
    // Create drive object for each drive.
    // TODO: this should only be base classes, ATAPIDrive should inherit from IDEDrive or something to support ATADrives as well.
    atapi_drives = new ATAPIDrive[n_found_drives];
    for (size_t i = 0; i < n_found_drives; i++)
    {
        int result = atapi_drives[i].populate_data(found_drives[i]);
        if (result < 0)
        {
            LOG("Error initiating ATAPI drive. Result: ", result);
            atapi_drives[i].drive_info = NULL;
            return result;
        }
    }
    return n_found_drives;
}
