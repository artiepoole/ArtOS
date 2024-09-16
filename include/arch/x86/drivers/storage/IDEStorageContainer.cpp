//
// Created by artypoole on 05/09/24.
//

#include "IDEStorageContainer.h"

#include <ATAPIDrive.h>
#include <Errors.h>
#include <IDE_DMA_PRDT.h>
#include <iso_fs.h>
#include <PIT.h>
#include <ports.h>
#include <stdlib.h>
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
        LOG("Setting PCI command bit 2 to true.");
        pci_dev->set_command_bit(2, true); // enable busmastering
        if (!(pci_dev->get_command() & 0x4))
        {
            LOG("Error enabling busmastering on PCI device.");
            return;
        }
    }
    BM_status_t bm_status = bm_dev->get_status();
    bm_status.error = 1;
    bm_status.interrupt = 1; //  writing 1 clears the bit
    bm_status = bm_dev->set_status(bm_status);
    if (bm_status.error == 1 || bm_status.interrupt == 1)
    {
        LOG("Resetting BM status didn't work.");
        return;
    }
    IDE_add_device(this); // add interrupt handling
    IDE_remove_device(this->drive_dev); // Stop interrupts from going directly to the contained device.
}

// Populates directory tree.
int IDEStorageContainer::mount()
{
    return populate_file_tree();
}

// Read from byte offset. Useful for use with files.
int IDEStorageContainer::read(void* dest, const size_t byte_offset, const size_t n_bytes)
{
    if (n_bytes == 0) return -1;
    if (n_bytes > 2048 * 32) return -1;
    const u32 lba_offset = byte_offset / drive_dev->drive_info->block_size;
    return read_lba(dest, lba_offset, n_bytes);
}


int IDEStorageContainer::prep_DMA_read(size_t lba_offset, size_t n_sectors)
{
    if (const int res = drive_dev->start_DMA_read(lba_offset, n_sectors); res != 0)
    {
        LOG("Error telling drive to prep for a DMA read");
        return res;
    }
    LOG("Drive ready for DMA read. Setting BM data flow direction.");
    auto cmd = bm_dev->get_cmd();
    cmd.rw_ctrl = DEV_TO_MEM;
    cmd = bm_dev->set_cmd(cmd); // apparently you set this then enable in separate writes.
    return 0;
}

void IDEStorageContainer::start_DMA_transfer()
{
    LOG("Starting DMA transfer.");
    BM_cmd_t bm_cmd = bm_dev->get_cmd();
    bm_cmd.start_stop = 1;

    BM_waiting_for_transfer = true;
    bm_dev->set_cmd(bm_cmd);
}

int IDEStorageContainer::wait_for_DMA_transfer() const
{
    LOG("Waiting for DMA transfer.");
    // TODO: This is not working properly when running full speed.
    BM_status_t bm_status = bm_dev->get_status();
    while (BM_waiting_for_transfer && bm_status.IDE_active && !bm_status.error)
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

int IDEStorageContainer::stop_DMA_read()
{
    LOG("Transfer complete. Resetting BM start/stop bit");
    BM_cmd_t bm_cmd = bm_dev->get_cmd();
    bm_cmd.start_stop = 0;
    bm_dev->set_cmd(bm_cmd);
    if (bm_dev->get_status().error)
    {
        return -DEVICE_ERROR;
    }
    return 0;
}

// Read from specified lba. useful internally. Might move to private
int IDEStorageContainer::read_lba(void* dest, size_t lba_offset, size_t n_bytes)
{
    const u16 n_sectors = (n_bytes + (drive_dev->drive_info->sector_size - 1)) / drive_dev->drive_info->sector_size; // round up division
    int ret_val = 0;
    ret_val = prep_DMA_read(lba_offset, n_sectors); // should set up ATA stuff and then set up BM stuff
    if (ret_val != 0) { return ret_val; }
    start_DMA_transfer(); // should just set BM start_stop
    ret_val = wait_for_DMA_transfer(); // should poll/wait/check status of each device.
    if (ret_val != 0) { return ret_val; }
    ret_val = stop_DMA_read(); // should just reset BM start_stop
    if (ret_val != 0) { return ret_val; }
    memcpy(dest, bm_dev->physical_region, n_bytes);
    return 0;
}

// Called by interrupt handler.
void IDEStorageContainer::notify()
{
    // LOG("IDEStorageContainer notified.");
    // Should just check if this controller/drive_dev was to be handled or not.
    if (!(BM_waiting_for_transfer || drive_dev->waiting_for_transfer)) return;

    BM_status_t bm_status = bm_dev->get_status();
    ATA_status_t ata_status = drive_dev->get_status();
    u8 ata_interrupt_reason = drive_dev->get_interrupt_reason();
#ifndef NDEBUG
    switch (ata_interrupt_reason)
    {
    case 0:
        {
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
    default:
        {
            break;
        }
    }
    if (ata_status.error)
    {
        // LOG("ATA device errored");
        drive_dev->waiting_for_transfer = false;
    }
#endif
    if (ata_status.data_request)
    {
        // LOG("ATA sent command");
        drive_dev->waiting_for_transfer = false;
    }
    else
    {
        // LOG("ATA probably didn't send command");
        BM_waiting_for_transfer = false;
    }

    if (bm_status.interrupt)
    {
        // LOG("BM interrupt raised.");
        bm_status.interrupt = true;
        bm_status = bm_dev->set_status(bm_status);
    }
}

/* convert an iso_directory_record_header into DirectoryData for use with ArtDirectory entries.*/
DirectoryData IDEStorageContainer::dir_record_to_directory(iso_directory_record_header& info, char*& name)
{
    return
        DirectoryData{
            this,
            info.extent_loc_LE,
            info.data_length_LE,
            tm{
                info.datetime.second,
                info.datetime.minute,
                info.datetime.hour,
                info.datetime.monthday,
                info.datetime.month,
                info.datetime.years_since_1900 + 1900,
                0,
                0,
                0
            },
            1,
            name
        };
}

/* convert an iso_directory_record_header into FileData for use with ArtDirectory entries.*/
FileData IDEStorageContainer::dir_record_to_file(const iso_directory_record_header& info, char*& name)
{
    return FileData{
        this,
        info.extent_loc_LE,
        info.data_length_LE,
        tm{
            info.datetime.second,
            info.datetime.minute,
            info.datetime.hour,
            info.datetime.monthday,
            info.datetime.month,
            info.datetime.years_since_1900 + 1900,
            0,
            0,
            0
        },
        1,
        name
    };
}

/* Loads the start of the volume descriptor from a CD ROM. Returns the iso_primary_volume_descriptor_t */
iso_primary_volume_descriptor_t IDEStorageContainer::get_primary_volume_descriptor()
{
    iso_primary_volume_descriptor_t vd{};
    constexpr size_t buf_size = sizeof(iso_primary_volume_descriptor_t);
    constexpr u32 data_start_lba = 16; // in LBA#
    read_lba(&vd, data_start_lba, buf_size);
    // TODO: ensure that the gotten vd is the primary vol desc.
    return vd;
}

/* Uses the primary volume descriptor to load the ISO 9660 path table and then uses the first path table entry to
 * locate the root dir in the ISO 9660 directory listing.
 * Returns the iso_path_table_entry_header for use with directory populating.
 */
iso_path_table_entry_header IDEStorageContainer::get_path_table_root_dir()
{
    // Load primary volume descriptor
    const iso_primary_volume_descriptor_t volume_descriptor = get_primary_volume_descriptor();

    // Load the path table
    char path_table_data[volume_descriptor.path_table_size_LE];
    read_lba(&path_table_data, volume_descriptor.path_l_table_loc_lba, volume_descriptor.path_table_size_LE);

    // get first dir entry. Ignore name because it is blank anyway.
    return *reinterpret_cast<iso_path_table_entry_header*>(path_table_data);
}

/* Given the lba
 *
 */
size_t IDEStorageContainer::get_dir_entry_size(ArtDirectory*& target_dir)
{
    constexpr size_t one_sector_size = 2048;
    char first_sector_data[one_sector_size];
    read_lba(&first_sector_data, target_dir->get_lba(), one_sector_size);
    return *reinterpret_cast<size_t*>(&first_sector_data[10]);
}

u8 IDEStorageContainer::populate_filename(char* sub_data, const u8 expected_name_length, const size_t ext_length, char*& filename)
{
    filename = strndup(sub_data, expected_name_length);
    if (strncmp(filename, "", expected_name_length) == 0 || strncmp(filename, "\1", expected_name_length) == 0)
    {
        free(filename);
        return 0;
    }
    size_t extension_pos = 0;
    while (extension_pos < ext_length)
    {
        if (sub_data[expected_name_length + extension_pos] == 0)
        {
            extension_pos++;
            continue;
        }
        auto [tag, len] = *reinterpret_cast<file_id_ext_header*>(&sub_data[expected_name_length + extension_pos]);
        if (strncmp(tag, "NM", 2) == 0)
        {
            free(filename);
            filename = strndup(&sub_data[expected_name_length + extension_pos + 5], len - 5);
            return len;
        }
        if (len > 0)
        {
            extension_pos += len;
        }
        else
        {
            return expected_name_length;
        }
    }
    return expected_name_length;
}

int IDEStorageContainer::populate_directory(ArtDirectory*& target_dir)
{
    size_t buffer_size = get_dir_entry_size(target_dir);
    if (buffer_size > 1024 * 64)
    {
        LOG("Cannot load full data without implementing larger physical region for DMA or handling partial transfers. Reading first 64k of data.");
        buffer_size = 1024 * 64;
    }
    char full_data[buffer_size];
    read_lba(&full_data, target_dir->get_lba(), buffer_size);
    size_t offset = 0;
    while (offset < buffer_size)
    {
        size_t start_offset = offset;
        auto dir_record = *reinterpret_cast<iso_directory_record_header*>(&full_data[offset]);
        if (dir_record.record_length == 0) // entries will not cross a sector boundary.
        {
            offset = offset + (2048 - (offset % 2048)); // skips remaining bytes to start of next sector
            continue;
        }
        offset += sizeof(iso_directory_record_header); // skip header to names
        char* filename;
        dir_record.file_name_length = populate_filename(
            &full_data[offset],
            dir_record.file_name_length,
            dir_record.record_length - sizeof(iso_directory_record_header),
            filename
        );
        if (dir_record.file_name_length == 0)
        {
            offset = start_offset + dir_record.record_length;
            continue;
        } // is either current or parent dir's entry or broken.;
        u32 data_len = dir_record.data_length_LE;
        u8 flags = dir_record.flags;
        LOG("Directory: ", target_dir->get_name(), " name: ", filename, " data length: ", data_len, " flags raw: ", flags, " is directory: ", bool(flags & 0x02));

        if (flags & 0x2)
        {
            target_dir->add_subdir(target_dir, dir_record_to_directory(dir_record, filename));
        }
        else
        {
            target_dir->add_file(dir_record_to_file(dir_record, filename));
        }

        offset = start_offset + dir_record.record_length;

        //todo: parse other extended file info tags.
        if (flags & 0x80) { LOG("Didn't read all extents for the previous file"); }
    }
    // todo: Error handling.
    return 0;
}

ArtDirectory* IDEStorageContainer::make_root_directory(const iso_path_table_entry_header& path_table_root)
{
    char first_sector_data[2048];
    read_lba(&first_sector_data, path_table_root.extent_loc, 2048);
    char* root_dir_name = strdup("/");
    auto dir_record = *reinterpret_cast<iso_directory_record_header*>(&first_sector_data[0]);
    return new ArtDirectory{
        nullptr,
        dir_record_to_directory(dir_record, root_dir_name),
    };
}

int IDEStorageContainer::populate_directory_recursive(ArtDirectory* target_dir)
{
    populate_directory(target_dir);
    auto device = this;
    target_dir->get_dirs()->iterate([device](ArtDirectory* dir) { device->populate_directory_recursive(dir); });
    // todo: Error handling.
    return 0;
}

int IDEStorageContainer::populate_file_tree()
{
    // todo refactor into void function?
    path_table_root = get_path_table_root_dir();
    root_directory = make_root_directory(path_table_root);
    populate_directory_recursive(root_directory);
    LOG("Directory Tree constructed.");
    return 0;
}
