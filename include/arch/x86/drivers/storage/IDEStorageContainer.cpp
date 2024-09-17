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
#include "Files.h"

constexpr size_t region_size = 65536;
#define one_sector_size this->drive_dev->drive_info->sector_size

#define MIN(X, Y) (((X) < (Y)) ? (X) : (Y))

IDEStorageContainer::IDEStorageContainer(ATAPIDrive* drive, PCIDevice* pci_dev, BusMasterController* bm_dev, const char* new_name): name(strdup(new_name))
{
    LOG("Initializing IDEStorageContainer");
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
    register_storage_device(this);
    LOG("IDEStorageContainer initialised.");

}

// Populates directory tree.
int IDEStorageContainer::mount()
{
    return populate_file_tree();
}

// Read from byte offset. Useful for use with files.
i64 IDEStorageContainer::read(char* dest, const size_t byte_offset, size_t n_bytes)
{
    if (n_bytes == 0) return 0;
    i64 n_read = 0;
    while (n_read < n_bytes)
    {
        // n_read has to be able to be neg but also up to max(u32) so use an i64. n_read >0 here due to program flow.
        size_t bytes_to_read = n_bytes - static_cast<u32>(n_read);
        if (bytes_to_read > region_size) { bytes_to_read = region_size; }
        const size_t lba_offset = (byte_offset + n_read) / drive_dev->drive_info->block_size;
        const size_t sector_offset = (byte_offset + n_read) % drive_dev->drive_info->block_size;
        const i64 res = read_lba(&dest[n_read], lba_offset, bytes_to_read, sector_offset);
        if (res == 0) { break; }
        if (res < 0) { return res; }
        n_read += res;
    }
    return n_read;
}

size_t IDEStorageContainer::get_block_size()
{
    return drive_dev->drive_info->block_size;
}

size_t IDEStorageContainer::get_block_count()
{
    return drive_dev->drive_info->capacity_in_LBA;
}

size_t IDEStorageContainer::get_sector_size()
{
    return drive_dev->drive_info->sector_size;
}


int IDEStorageContainer::prep_DMA_read(size_t lba_offset, size_t n_sectors)
{
    if (const int res = drive_dev->start_DMA_read(lba_offset, n_sectors); res != 0)
    {
        LOG("Error telling drive to prep for a DMA read");
        return res;
    }
    auto cmd = bm_dev->get_cmd();
    cmd.rw_ctrl = DEV_TO_MEM;
    cmd = bm_dev->set_cmd(cmd); // apparently you set this then enable in separate writes.
    return 0;
}

void IDEStorageContainer::start_DMA_transfer()
{
    BM_cmd_t bm_cmd = bm_dev->get_cmd();
    bm_cmd.start_stop = 1;

    BM_waiting_for_transfer = true;
    bm_dev->set_cmd(bm_cmd);
}

int IDEStorageContainer::wait_for_DMA_transfer() const
{
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
auto IDEStorageContainer::read_lba(void* dest, size_t lba_offset, const size_t n_bytes, const size_t sector_offset) -> i64
{
    // trim oversize reads
    u16 n_sectors = ((n_bytes + sector_offset) + (drive_dev->drive_info->sector_size - 1)) / drive_dev->drive_info->sector_size & 0xffff; // round up division
    if (n_sectors > 32) { n_sectors = 32; }

    // put data in physical reagion
    int ret_val = 0;
    ret_val = prep_DMA_read(lba_offset, n_sectors); // should set up ATA stuff and then set up BM stuff
    if (ret_val != 0) { return ret_val; }
    start_DMA_transfer(); // should just set BM start_stop
    ret_val = wait_for_DMA_transfer(); // should poll/wait/check status of each device.
    if (ret_val != 0) { return ret_val; }
    ret_val = stop_DMA_read(); // should just reset BM start_stop
    if (ret_val != 0) { return ret_val; }
    // handle copy
    const i64 actual_bytes = MIN(n_bytes, n_sectors*one_sector_size - sector_offset);
    memcpy(dest, &bm_dev->physical_region[sector_offset], static_cast<size_t>(actual_bytes));
    return actual_bytes;
}

// Called by interrupt handler.
void IDEStorageContainer::notify()
{
    // LOG("IDEStorageContainer notified.");
    // Should just check if this controller/drive_dev was to be handled or not.
    if (!(BM_waiting_for_transfer || drive_dev->waiting_for_transfer)) return;

    BM_status_t bm_status = bm_dev->get_status();
    const ATA_status_t ata_status = drive_dev->get_status();
#ifndef NDEBUG
    switch (const u8 ata_interrupt_reason = drive_dev->get_interrupt_reason(); ata_interrupt_reason)
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

ArtFile* IDEStorageContainer::find_file(const char* filename)
{
    return root_directory->search_recurse(filename);
}

/* convert an iso_directory_record_header into DirectoryData for use with ArtDirectory entries.*/
DirectoryData IDEStorageContainer::dir_record_to_directory(const iso_directory_record_header& info, char*& name)
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
    read_lba(&vd, data_start_lba, buf_size, 0);
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
    read_lba(&path_table_data, volume_descriptor.path_l_table_loc_lba, volume_descriptor.path_table_size_LE, 0);

    // get first dir entry. Ignore name because it is blank anyway.
    return *reinterpret_cast<iso_path_table_entry_header*>(path_table_data);
}

/* Given the lba
 *
 */
size_t IDEStorageContainer::get_dir_entry_size(ArtDirectory*& target_dir)
{
    char first_sector_data[this->drive_dev->drive_info->sector_size];
    read_lba(&first_sector_data, target_dir->get_lba(), one_sector_size, 0);
    return *reinterpret_cast<size_t*>(&first_sector_data[10]);
}

u8 IDEStorageContainer::populate_filename(char* sub_data, const u8 expected_name_length, const size_t ext_length, char*& filename) const
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
    if (buffer_size > region_size)
    {
        LOG("Cannot load full data without implementing larger physical region for DMA or handling partial transfers. Reading first 64k of data.");
        buffer_size = region_size;
    }
    char full_data[buffer_size];
    read_lba(&full_data, target_dir->get_lba(), buffer_size, 0);
    size_t offset = 0;
    while (offset < buffer_size)
    {
        const size_t start_offset = offset;
        /*
         *  If there are more entries after this one but which start on the next sector, the sector will be padded with zeros.
         *  If the first value (record length) is zero, we skip to the first byte of the next sector. If this is out of bounds,
         *  the loop ends, otherwise the reading continues.
         */
        if (full_data[offset] == 0)
        {
            offset = offset + (one_sector_size - (offset % one_sector_size)); // skips remaining bytes to start of next sector
            continue;
        }
        auto dir_record = *reinterpret_cast<iso_directory_record_header*>(&full_data[offset]);


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
        [[maybe_unused]] u32 data_len = dir_record.data_length_LE;
        u8 flags = dir_record.flags;

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

ArtDirectory* IDEStorageContainer::make_root_directory(const iso_path_table_entry_header& p_t_r)
{
    char first_sector_data[one_sector_size];
    read_lba(&first_sector_data, p_t_r.extent_loc, one_sector_size, 0);
    char* root_dir_name = strdup("/");
    const auto dir_record = *reinterpret_cast<iso_directory_record_header*>(&first_sector_data[0]);
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
    LOG("Constructing CD ROM directory tree");
    const auto path_table_root = get_path_table_root_dir();
    root_directory = make_root_directory(path_table_root);
    populate_directory_recursive(root_directory);
    LOG("Directory Tree constructed.");
    return 0;
}
