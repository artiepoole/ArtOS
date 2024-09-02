//
// Created by artypoole on 27/08/24.
//

#include "ATA.h"

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

// ATAPI packet opcodes
// #define ATAPI_READ_CD 0x80
#define ATAPI_INQUIRY_CMD 0x12
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
#define PRIMARY_DRIVE_ADDR_PORT 0x3F7

#define SECONDARY_ALT_STATUS_PORT 0x376
#define SECONADARY_DRIVE_ADDR_PORT 0x377


struct ATAPI_inquiry_data_t
{
    u8 peripheral_type;
    u8 RMB;
    u8 version;
    u8 response_data_format;
    u8 addnl_length; ///< n - 4, Numbers of bytes following this one.
    u8 reserved_5;
    u8 reserved_6;
    u8 reserved_7;
    u8 vendor_info[8];
    u8 product_id[16];
    u8 product_revision_level[4];
};


union ATAPI_packet_t
{
    struct
    {
        u8 opcode;
        u8 device;
        u8 lba_high;
        u8 lba_mid;
        u8 lba_low;
        u8 sector_count;
        u8 control;
        u8 res1[5];
    };

    u16 words[6];
};

struct ATAPI_ext_packet_t
{
    u8 opcode;
    u8 feature;
    u8 sector_count;
    u8 lba_low;
    u8 lba_mid;
    u8 lba_high;
    u8 drive_sel;
    u8 res0;
    u8 control;
    u8 res1[3];
    u8 additional[4];
};

u16 ata_base_port;
u16 is_primary_ATA;
u16 is_main_ATA;
volatile bool ATA_transfer_in_progress = false;


/* To test if atapi supported:
 * set DRIVE_SEL
 * wait til not busy
 * write 0xA1 (ident) to CMD
 * read STATUS (bit 3 : rdy but not Bit 0: err)
 * if true, read 512 bytes and inspect it.
 */


ATA_status_t get_ata_device_status()
{
    ATA_status_t status;
    status.raw = inb(ata_base_port + STATUS_OFFSET);
    return status;
}

ATA_status_t get_ata_alt_status()
{
    ATA_status_t status;
    if (is_primary_ATA)
    {
        status.raw = inb(PRIMARY_ALT_STATUS_PORT);
    }
    else
    {
        status.raw = inb(SECONDARY_ALT_STATUS_PORT);
    }
    return status;
}


ATA_error_t get_ata_device_error()
{
    ATA_error_t error;
    error.raw = inb(ata_base_port + ERROR_OFFSET);
    return error;
}

u8 get_ata_interrupt_reason()
{
    return inb(ata_base_port + INTERRUPT_REASON_OFFSET);
}

u16 get_n_bytes_to_read()
{
    u16 port;
    if (is_primary_ATA) port = PRIMARY_BASE_PORT;
    else port = SECONDARY_BASE_PORT;

    return inb(port + LBA_MID_OFFSET) | inb(port + LBA_HIGH_OFFSET) << 8;
}

void poll_busy()
{
    ATA_status_t status = get_ata_alt_status();
    while (status.busy & !status.error)
    {
        status = get_ata_alt_status();
    }
}

void poll_until_DRQ()
{
    ATA_status_t status = get_ata_alt_status();
    while (!status.data_request & !status.error)
    {
        status = get_ata_alt_status();
    }
}

/* Finds first drive
 * return:
 * if val = 0xFF, no device detected
 * bit 0: primary if set, secondary if not.
 * bit 1: main drive if set, alt drive if not
 * bits 2-31, not used
 * val <0: error
 */
IDE_drive_t find_drive()
{
    u16 ports[2] = {PRIMARY_BASE_PORT, SECONDARY_BASE_PORT};
    u16 drives[2] = {MAIN_DRIVE_SEL_CMD, ALT_DRIVE_SEL_CMD};
    for (u8 port = 0; port < 2; port++)
    {
        for (u8 drive = 0; drive < 2; drive++)
        {
            outb(ports[port] + DRIVE_SEL_OFFSET, drives[drive]);

            // Wait 400ns for the drive to switch (reading status 4 times is a common way)
            sleep(1);

            outb(port + CMD_OFFSET, ATA_IDENT_CMD);
            while (inb(ports[port] + STATUS_OFFSET) & STATUS_BSY)
            {
            }

            ATA_status_t status{};
            status.raw = inb(ports[port] + STATUS_OFFSET);

            if (status.ready)
            {
                return IDE_drive_t{
                    true,
                    static_cast<bool>(port),
                    static_cast<bool>(drive),
                    false,
                    false
                };
            }
            if (status.error)
            {
                LOG("Device ERROR detected. Primary IDE:", port == 1, " Main drive: ", drive == 1);
            }
        }
    }
    return IDE_drive_t{}; // no drives
}

/* only sends packet, doesn't send the interpret packet command */
int send_packet_exepct_PIO(const ATAPI_packet_t& packet)
{
    poll_busy();

    // Transfer waits for irq. IRQ just resets transfer in progress
    ATA_transfer_in_progress = true;
    LOG("Sending PACKET command for PIO");
    outb(ata_base_port + CMD_OFFSET, PROCESS_PACKET_CMD);
    // wait for ready
    poll_busy();
    poll_until_DRQ();
    LOG("Sending packet data");
    // send data
    for (int i = 0; i < 6; i++)
    {
        outw(ata_base_port + DATA_OFFSET, packet.words[i]);
    }
    LOG("Packet data sent. Is in progress:", ATA_transfer_in_progress);
    // poll_busy();

    // wait for done before reading.
    ATA_status_t status = get_ata_alt_status();
    if (status.data_request)
    {
        LOG("DEVICE STILL EXPECTING DATA?");
        // while (status.data_request)
        // {
        //     outw(ata_base_port + DATA_OFFSET, 0);
        //     status = get_ata_alt_status();
        // }
    }
    while (ATA_transfer_in_progress & !status.error & !status.device_fault)
    {
        status = get_ata_alt_status();
    }

    if (ATA_transfer_in_progress)
    {
        ATA_error_t error = get_ata_device_error();
        LOG("Error enabling DMA. Raw error", error.raw);
        ATA_transfer_in_progress = false;
        return -DEVICE_ERROR;
    }
    return 0;
}

int send_packet_for_DMA(const ATAPI_packet_t& packet)
{
    LOG("Sending DMA read request PACKET command");
    poll_busy();
    outb(ata_base_port + CMD_OFFSET, PROCESS_PACKET_CMD);
    // wait for ready
    poll_busy();
    poll_until_DRQ();
    LOG("Sending DMA packet DATA");
    // send data
    for (int i = 0; i < 6; i++)
    {
        outw(ata_base_port + DATA_OFFSET, packet.words[i]);
    }

    // wait for busy and not errors before continuing.
    ATA_status_t status = get_ata_alt_status();
    while (!(status.error | status.device_fault | status.busy))
    {
        status = get_ata_alt_status();
    }
    if (status.error | status.device_fault)
    {
        LOG("Error requesting DMA");
        return -DEVICE_ERROR;
    }
    return 0;
}


/* Returns 1 if ATAPI, 0 if ATA, < 0 for errors */
int ATA_test_if_ATAPI()
{
    // send request
    outb(ata_base_port + CMD_OFFSET, ATAPI_IDENT_CMD);
    poll_busy();
    poll_until_DRQ();

    ATA_status_t status = get_ata_device_status();
    if (status.data_request and ~(status.error)) // data request is true and error is false
    {
        //todo: test whether you can skip reading all we don't use
        u16 identity_data[256];
        for (unsigned short& i : identity_data)
        {
            i = inw(ata_base_port + DATA_OFFSET);
        }

        if (identity_data[0] == 0x848A or identity_data[0] == 0xC800)
        {
            LOG("ATAPI DEVICE CONFIRMED");
            return true;
        }
        if (identity_data[0] == 0x848A or identity_data[0] == 0xC800)
        {
            LOG("ATAPI DEVICE CONFIRMED");
            return true;
        }
        if (identity_data[0] == 0x85c0)
        {
            /* raw binary gives
             * 10 == atapi confirmed
             * 0 - res
             * 00101 - command packet set supported by device >> 0x5 means CD rom set
             * 1 - removable
             * 10 == accelerated DRQ
             * 000 // res
             * 00 == 12 byte cmd packet
             */
            LOG("ATAPI DEVICE CONFIRMED. 12 byte cmd packets.");
            LOG(identity_data[62]);

            return true;
        }
        return -DEVICE_ERROR;
    }

    ATA_error_t error = get_ata_device_error();
    LOG("ERROR:", error.raw);

    return -DEVICE_ERROR;
}


int ATAPI_init(IDE_drive_t& drive)
{
    LOG("Initialising ATAPI device.");

    is_primary_ATA = !drive.controller_id;
    is_main_ATA = !drive.drive_id;
    if (is_primary_ATA)
    {
        ata_base_port = PRIMARY_BASE_PORT;
    }
    else
    {
        ata_base_port = SECONDARY_BASE_PORT;
    }
    // select drive
    //  http://users.utcluj.ro/~baruch/media/siee/labor/ATA-Interface.pdf
    u8 drive_data;
    if (is_main_ATA) drive_data = MAIN_DRIVE_SEL_CMD;
    else drive_data = ALT_DRIVE_SEL_CMD;

    outb(ata_base_port + DRIVE_SEL_OFFSET, drive_data);
    poll_busy();

    outb(ata_base_port + CONTROL_OFFSET, 0x00);
    poll_busy();

    // enable DMA mode
    // Set Features to enable DMA mode
    outb(ata_base_port + FEATURES_OFFSET, TRANSFER_MODE); // Subcommand to set transfer mode
    outb(ata_base_port + SECTOR_COUNT_OFFSET, 0x03); // Set DMA mode 1 (or use 0x20, 0xC5, etc. for other modes)
    outb(ata_base_port + CMD_OFFSET, SET_FEATURES_CMD); // Send the Set Features command
    if (ATA_status_t status = get_ata_alt_status(); status.error)
    {
        ATA_error_t error = get_ata_device_error();
        LOG("Error enabling DMA. Raw error", error.raw);
        return -DEVICE_ERROR;
    }
    drive.dma_capable = true;

    if (ATA_test_if_ATAPI() > 0)
    {
        drive.atapi_capable = true;
        return 0;
    }
    return -DEVICE_ERROR;
}


int ATAPI_start_DMA_read(const u16 n_sectors)
{
    // create SCSI packet
    ATAPI_packet_t packet = {};
    packet.opcode = 0x28;
    packet.words[4] = n_sectors & 0xFFFF;


    // Flush buffers and set features to request DMA transfer for read data
    outb(ata_base_port + FEATURES_OFFSET, 0x01); // Set DMA mode for packet transfer
    outb(ata_base_port + LBA_LOW_OFFSET, 0);
    outb(ata_base_port + LBA_MID_OFFSET, 0);
    outb(ata_base_port + LBA_HIGH_OFFSET, 0);
    outb(ata_base_port + DRIVE_SEL_OFFSET, 0); // main drive
    outb(ata_base_port + SECTOR_COUNT_OFFSET, 0);

    // Send the packet
    if (send_packet_for_DMA(packet) != 0)
    {
        LOG("Error reading using DMA");
        return -DEVICE_ERROR;
    }

    return 0;
}


u32 ATAPI_get_capacity()
{
    ATAPI_packet_t packet = {0};
    packet.opcode = ATAPI_GET_CAPACITY_CMD;

    // "FLUSH" buffers.
    outb(ata_base_port + FEATURES_OFFSET, 0); // don't use DMA for this
    outb(ata_base_port + LBA_LOW_OFFSET, 0); // Byte count request.
    outb(ata_base_port + LBA_MID_OFFSET, 12);
    outb(ata_base_port + LBA_HIGH_OFFSET, 0);

    // Send the packet.
    send_packet_exepct_PIO(packet);

    if (ATA_status_t status = get_ata_alt_status(); status.error)
    {
        ATA_error_t error = get_ata_device_error();
        LOG("Error getting capacity. Raw error: ", error.raw);
        return 0;
    }

    u16 n_bytes = get_n_bytes_to_read();

    u16 data[n_bytes / 2] = {0};
    for (u16& i : data)
    {
        i = inw(ata_base_port + DATA_OFFSET);
    }
    // big endian
    u32 max_lba = data[0] << 16 | data[1];
    u32 block_size = data[2] << 16 | data[3];
    return max_lba;
}
