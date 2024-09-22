//
// Created by artypoole on 05/09/24.
//

#ifndef IDE_DEVICE_H
#define IDE_DEVICE_H

#include <ArtDirectory.h>
#include <iso_fs.h>

#include "Errors.h"

#include "StorageDevice.h"

#include "types.h"

#include "BusMasterController.h"
#include "PCIDevice.h"
#include "IDE_notifiable.h"
#include "IDEDrive.h"

typedef int (*readFunc)();
typedef int (*seekFunc)();
typedef int (*writeFunc)();

class IDEStorageContainer : public IDE_notifiable, public StorageDevice
{
public:
    IDEStorageContainer(IDE_drive_info_t& drive_info, PCIDevice* pci_dev, BusMasterController* bm_dev, const char* new_name);
    ~IDEStorageContainer() override = default; // TODO: remove PRDT?

    int mount() override;

    i64 read(char* dest, size_t byte_offset, size_t n_bytes) override;
    i64 read(void* dest, size_t byte_offset, size_t n_bytes);
    i64 read_lba(void* dest, size_t lba_offset, size_t n_bytes);
    i64 seek([[maybe_unused]] u64 offset, [[maybe_unused]] int whence) override { return -NOT_IMPLEMENTED; }
    i64 write([[maybe_unused]] const char* src, [[maybe_unused]] size_t byte_count, [[maybe_unused]] size_t byte_offset) override { return -NOT_IMPLEMENTED; }
    char* get_name() override {return name;}

    size_t get_block_size() override;
    size_t get_block_count() override;
    size_t get_sector_size() override;

    int prep_DMA_read(size_t lba, size_t n_sectors);
    void start_DMA_transfer();
    [[nodiscard]] int wait_for_DMA_transfer() const;
    int stop_DMA_read();
    int read_into_region_from_lba(size_t lba_offset);


    void notify() override;


    ArtFile* find_file(const char* filename) override;

private:
    // priavte member functions
    DirectoryData dir_record_to_directory(const iso_directory_record_header& info, char*& name);
    FileData dir_record_to_file(const iso_directory_record_header& info, char*& name);

    iso_path_table_entry_header get_path_table_root_dir();
    iso_primary_volume_descriptor_t get_primary_volume_descriptor();
    ArtDirectory* make_root_directory(const iso_path_table_entry_header& p_t_r);

    size_t get_dir_entry_size(ArtDirectory*& target_dir);
    u8 populate_filename(char* sub_data, u8 expected_name_length, size_t ext_length, char*& filename) const;
    int populate_directory(ArtDirectory*& target_dir);
    int populate_directory_recursive(ArtDirectory* target_dir);
    int populate_file_tree();

    // members
    char *name;
    IDEDrive* drive_dev;
    PCIDevice* pci_dev;
    BusMasterController* bm_dev;
    ArtDirectory* root_directory = nullptr;
    volatile bool BM_waiting_for_transfer = false; // todo private member
    i64 stored_buffer_start = -1;
};

#endif //IDE_DEVICE_H
