//
// Created by artypoole on 27/08/24.
//

#include "IDE.h"

#include <Errors.h>
#include <IDT.h>
#include <logging.h>
#include <PIT.h>
#include <ports.h>

#include "ATA.h"
#include "string.h"

#include "stdlib.h"
#include "../../../../../../../../../usr/lib/gcc/x86_64-linux-gnu/9/include/float.h"

// http://www.osdever.net/downloads/docs/idems100.zip
#define PRIMARY_CMD_OFFSET 0x00
#define PRIMARY_STATUS_OFFSET 0x02
#define PRIMARY_PRDT_START_OFFSET  0x04

#define SECONDARY_CMD_OFFSET 0x08
#define SECONDARY_STATUS_OFFSET 0x0A
#define SECONDARY_PRDT_START_OFFSET 0x0C

//command settings

#define DEV_TO_MEM 0x01;
#define MEM_TO_DEV 0x00;
#define DMA_MODE 0x08;

#define PRDT_SIZE 65536

u8 physical_region[PRDT_SIZE] __attribute__((aligned(1024 * 64)));
PRDT_t table{};

u16 bus_master_base_port;
u16 is_primary_IDE;
u16 is_main_IDE;

BM_status_t last_bus_master_status{};
u8 last_atapi_status;


volatile bool BM_transfer_in_progress = false;
// todo: there should be a DMA transfer in progress and an ATA transfer in progress.

// void write_bus_master_ide(void* base_addr, const u8 offset, const u8 data)
// {
//     auto addr = static_cast<u8*>(base_addr + offset);
//     *addr = data;
// }

// u8 read_bus_master_ide(void* base_addr, const u8 offset)
// {
//     return *static_cast<u8*>(base_addr + offset);
// }
//  https://forum.osdev.org/viewtopic.php?t=19056
u8* DMA_init_PRDT()
{
    u16 prdt_offset;
    if (is_primary_IDE)
    {
        prdt_offset = PRIMARY_PRDT_START_OFFSET;
    }
    else
    {
        prdt_offset = SECONDARY_PRDT_START_OFFSET;
    }


    table.descriptor.base_addr = reinterpret_cast<u32>(physical_region) & 0xFFFFFFF0;
    table.descriptor.length_in_b = 0;
    table.descriptor.end_of_table = 1;

    const auto table_loc = reinterpret_cast<u32>(&table.descriptor) & 0xFFFFFFF0; // 2 reserved bits at bit 0 and 1

    outw(bus_master_base_port + prdt_offset, table_loc & 0xFFFF); // low bytes
    outw(bus_master_base_port + prdt_offset + 2, (table_loc >> 16) & 0xFFFF); // high bytes

    u8 top = inb(bus_master_base_port + prdt_offset + 3);
    if (top != table_loc >> 24 & 0xFF)
    {
        LOG("Error setting physical region");
        return nullptr;
    }
    return physical_region;
}

void DMA_free_prdt()
{
    // todo: remove prdt and prd.

    free(physical_region);
}


BM_status_t get_busmaster_status()
{
    BM_status_t status{};
    if (is_primary_IDE)
    {
        status.raw = inb(bus_master_base_port + PRIMARY_STATUS_OFFSET);
    }
    else
    {
        status.raw = inb(bus_master_base_port + SECONDARY_STATUS_OFFSET);
    }
    return status;
}

BM_status_t set_busmaster_status(BM_status_t status)
{
    if (is_primary_IDE)
    {
        outb(bus_master_base_port + PRIMARY_STATUS_OFFSET, status.raw);
    }
    else
    {
        outb(bus_master_base_port + SECONDARY_STATUS_OFFSET, status.raw);
    }
    return get_busmaster_status();
}

BM_cmd_t get_busmaster_command()
{
    BM_cmd_t cmd;
    if (is_primary_IDE)
    {
        cmd.raw = inb(bus_master_base_port + PRIMARY_CMD_OFFSET);
    }
    else
    {
        cmd.raw = inb(bus_master_base_port + SECONDARY_CMD_OFFSET);
    }
    return cmd;
}

BM_cmd_t set_busmaster_command(BM_cmd_t cmd)
{
    if (is_primary_IDE)
    {
        outb(bus_master_base_port + PRIMARY_CMD_OFFSET, cmd.raw);
    }
    else
    {
        outb(bus_master_base_port + SECONDARY_CMD_OFFSET, cmd.raw);
    }
    return get_busmaster_command();
}


int init_busmaster_device(u16 new_base_port, IDE_drive_t& drive)
{
    LOG("Initialising busmaster device.");
    bus_master_base_port = new_base_port;
    is_primary_IDE = !drive.controller_id;
    is_main_IDE = !drive.drive_id;

    BM_status_t status = get_busmaster_status();
    status.error = 1;
    status.interrupt = 1; //  writing 1 clears the bit
    status.drive_0_dma_capable = 1;
    status = set_busmaster_status(status);

    if (status.error)
    {
        LOG("Error when initialising busmaster device to use DMA on device 1 and IDE active.");
        return -DEVICE_ERROR;
    }

    if (DMA_init_PRDT() == nullptr)
    {
        LOG("Error setting PRDT. Aborting.");
        return -DEVICE_ERROR;
    }

    return 0;
}

void DMA_read(u8* dest, const size_t n_bytes)
{
    if (n_bytes == 0) return;
    u16 n_sectors = (n_bytes + (sector_size - 1)) / sector_size; // round up division

    ATAPI_start_DMA_read(n_sectors);
    LOG("Reading ", n_sectors, " sectors.");
    auto cmd = get_busmaster_command();
    cmd.rw_ctrl = DEV_TO_MEM;
    cmd = set_busmaster_command(cmd); // apparently you set this then enable in separate writes.
    BM_status_t status = get_busmaster_status();
    cmd = get_busmaster_command();
    LOG("Setting BM transfer in progress.");
    BM_transfer_in_progress = true;
    cmd.start_stop = 1;
    cmd = set_busmaster_command(cmd);
    status = get_busmaster_status();
    while (BM_transfer_in_progress & status.IDE_active)
    {
        status = get_busmaster_status();
    }
    if (BM_transfer_in_progress) { LOG("BM transfer complete didn't send interrupt?"); }
    else
    {
        cmd.start_stop = 0;
        cmd = set_busmaster_command(cmd);
    }
    memcpy(dest, physical_region, n_bytes);
}


void IDE_handler(const bool from_primary)
{
    // set command start/stop bit correctly
    if (from_primary)
        LOG("IRQ 14 FIRED");
    else
        LOG("IRQ 15 FIRED");

    BM_status_t bm_status = get_busmaster_status();
    ATA_status_t ata_status = get_ata_device_status();

    if (ata_status.error)
    {
        LOG("ATA device errored");
        ATA_transfer_in_progress = false;
    }

    if (ata_status.data_request)
    {
        LOG("ATA sent command");
        ATA_transfer_in_progress = false;
    }
    else
    {
        LOG("ATA probably didn't send command");
        BM_transfer_in_progress = false;
    }

    if (bm_status.interrupt)
    {
        LOG("BM interrupt raised.");
        bm_status.interrupt = true;
        bm_status = set_busmaster_status(bm_status);
    }
}
