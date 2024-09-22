//
// Created by artypoole on 05/09/24.
//

#include "ATAPIDrive.h"

#include "Errors.h"
#include "logging.h"
#include "ports.h"
#include "ATA.h"


// ATAPI exclusive commands
#define ATAPI_INQUIRY_CMD 0x12
#define ATAPI_GET_CAPACITY_CMD 0x25
#define ATAPI_IDENT_CMD 0xA1
#define PROCESS_PACKET_CMD 0xA0

// ATAPI command opcodes
#define ATAPI_INQUIRY_CMD 0x12
#define READ_10_CMD 0x28

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

// Predefined register states
ATAPI_cmd_regs get_capacity_cmd_regs{0, 0, 0, 8, 0}; // LBA_low may need to be 12. Not sure if n bytes requested or n bytes sent.
ATAPI_cmd_regs init_dma_cmd_regs{0x1, 0, 0, 0, 0}; //

ATAPIDrive::ATAPIDrive(IDE_drive_info_t& drive_info)
{
    this->drive_info = &drive_info;
}

ATAPIDrive::~ATAPIDrive() = default;

int ATAPIDrive::populate_data()
{

    waiting_for_transfer = true;
    LOG("Initialising drive");
    if (populate_capabilities() < 0)
    {
        LOG("Initialisation failed. Aborting.");
        return -DEVICE_ERROR;
    }


    return 0;
}

int ATAPIDrive::populate_capabilities()
{
    switch (ATA_is_packet_device(drive_info))
    {
    case 0:
        {
            // TODO: Not implemented yet
            ATA_ident(drive_info, identity_data);
            return 0;
        }
    case 1:
        {
            if (const int res = ATAPI_ident(drive_info, identity_data); res < 0) return -DEVICE_ERROR;
            // Choose DMA capabilities
            if (drive_info->DMA_device)
            {
                if (drive_info->UDMA_modes & 1)
                {
                    ATAPI_set_max_dma_mode(true, drive_info);
                }
                else if (drive_info->MW_DMA_modes & 1)
                {
                    ATAPI_set_max_dma_mode(false, drive_info);
                }
            }
            ATA_status_t status = get_status();
            if (status.error || status.device_fault) return -DEVICE_ERROR;
            drive_info->capacity_in_LBA = get_last_lba();
            status = get_status();
            if (status.error || status.device_fault)
            {
                LOG("Error getting capacity. raw error: ", get_error().raw);
                return -DEVICE_ERROR;
            }
            return 0;
        }
    default:
        return -DEVICE_ERROR;
    }
}

int ATAPIDrive::start_DMA_read(u32 lba_offset, const size_t n_sectors)
{
    // create SCSI packet
    ATAPI_packet_t packet = {};
    packet.bytes[0] = READ_10_CMD;
    packet.bytes[2] = lba_offset >> 24 & 0xFF;
    packet.bytes[3] = lba_offset >> 16 & 0xFF;
    packet.bytes[4] = lba_offset >> 8 & 0xFF;
    packet.bytes[5] = lba_offset & 0xFF;
    packet.bytes[8] = n_sectors & 0xFF;
    packet.bytes[7] = n_sectors >> 8 & 0xFF;

    // Flush buffers and set features to request DMA transfer for read data
    set_regs(init_dma_cmd_regs);

    // Send the packet
    if (send_packet_DMA(packet) != 0)
    {
        LOG("Error reading using DMA");
        return -DEVICE_ERROR;
    }


    return 0;
}

int ATAPIDrive::seek(size_t LBA)
{
    return -NOT_IMPLEMENTED;
}

int ATAPIDrive::start_DMA_write(u32 lba, size_t n_sectors)
{
    return -NOT_IMPLEMENTED;
}

int ATAPIDrive::set_regs(const ATAPI_cmd_regs& regs)
{
    for (size_t i = 0; i < sizeof(ATAPI_cmd_regs); i++)
    {
        outb(drive_info->base_port + FEATURES_OFFSET + i, regs.bytes[i]);
    }

    ATA_poll_busy(drive_info);
    if (const ATA_status_t status = get_alt_status(); status.error || status.device_fault)
    {
        return -DEVICE_ERROR;
    }
    return 0;
}


u32 ATAPIDrive::get_last_lba()
{
    ATA_select_drive(drive_info);
    ATAPI_packet_t packet = {0};
    packet.bytes[0] = ATAPI_GET_CAPACITY_CMD;

    // "FLUSH" buffers.
    set_regs(get_capacity_cmd_regs);

    // Send the packet.
    int result = send_packet_PIO(packet);
    if (result != 0)
    {
        return -DEVICE_ERROR;
    }
    if (ATA_status_t status = get_alt_status(); status.error)
    {
        [[maybe_unused]] ATA_error_t error = get_error();
        LOG("Error getting capacity. Raw error: ", error.raw);
        return 0;
    }

    u16 n_bytes = ATA_get_n_bytes_to_read(drive_info);

    u16 data[(n_bytes + 1) / 2] = {0};
    for (u16& i : data)
    {
        i = inw(drive_info->base_port + DATA_OFFSET);
    }

    // big endian
    const u32 max_lba = data[0] << 16 | data[1];
    // u32 last_block_size = data[2] << 16 | data[3];
    return max_lba;
}


int ATAPIDrive::send_packet_PIO(const ATAPI_packet_t& packet)
{
    ATA_poll_busy(drive_info);

    // Transfer waits for irq. IRQ just resets transfer in progress
    waiting_for_transfer = true;
    outb(drive_info->base_port + CMD_OFFSET, PROCESS_PACKET_CMD);
    // wait for ready
    ATA_poll_busy(drive_info);
    ATA_poll_until_DRQ(drive_info);
    // send data
    for (unsigned short word : packet.words)
    {
        outw(drive_info->base_port + DATA_OFFSET, word);
    }

    // wait for done before reading.
    ATA_status_t status = get_alt_status();
    while (waiting_for_transfer && !status.error && !status.device_fault)
    {
        status = get_alt_status();
    }

    if (waiting_for_transfer)
    {
        [[maybe_unused]] ATA_error_t error = get_error();
        LOG("Error sending packet using PIO. Raw error", error.raw);
        waiting_for_transfer = false;
        return -DEVICE_ERROR;
    }
    return 0;
}


int ATAPIDrive::send_packet_DMA(const ATAPI_packet_t& packet)
{
    ATA_select_drive(drive_info);
    ATA_poll_busy(drive_info);
    outb(drive_info->base_port + CMD_OFFSET, PROCESS_PACKET_CMD);
    // waiting_for_transfer = true;
    ATA_poll_busy(drive_info);
    ATA_poll_until_DRQ(drive_info);
    // send data
    for (unsigned short word : packet.words)
    {
        outw(drive_info->base_port + DATA_OFFSET, word);
    }
    // wait for busy and not errors before continuing.
    ATA_status_t status = get_alt_status();
    while (!(status.error | status.device_fault | status.data_request))
    {
        status = get_alt_status();
    }
    if (status.error | status.device_fault)
    {
        LOG("Error requesting DMA");
        return -DEVICE_ERROR;
    }
    return 0;
}



