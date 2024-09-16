//
// Created by artypoole on 05/09/24.
//

#ifndef IDE_DEVICE_H
#define IDE_DEVICE_H

#include <ArtDirectory.h>
#include <iso_fs.h>

#include "Errors.h"

#include "ATAPIDrive.h"
#include "StorageDevice.h"

#include "types.h"

#include "BusMasterController.h"
#include "PCIDevice.h"
#include "IDE_notifiable.h"

typedef int (*readFunc)();
typedef int (*seekFunc)();
typedef int (*writeFunc)();

class IDEStorageContainer: public IDE_notifiable, public StorageDevice
{
public:
    IDEStorageContainer(ATAPIDrive* drive, PCIDevice* pci_dev, BusMasterController* bm_dev);
    int read_lba(void* dest, size_t lba_offset, size_t n_bytes);
    ~IDEStorageContainer() override = default; // TODO: remove PRDT?
    int read(void* dest, size_t byte_offset, size_t n_bytes) override;
    int seek(size_t offset, int whence) override {return -NOT_IMPLEMENTED;}
    int write(void* src, size_t byte_count, size_t byte_offset) override {return -NOT_IMPLEMENTED;}
    int mount();


    int load_file(char* filename);
    void notify() override;
    int prep_DMA_read(size_t lba, size_t n_sectors);

    void start_DMA_transfer();
    int wait_for_DMA_transfer() const;
    int stop_DMA_read();
    DirectoryData dir_record_to_directory(iso_directory_record_header& info, char*& name);
    FileData dir_record_to_file(const iso_directory_record_header& info, char*& name);
    iso_primary_volume_descriptor_t get_primary_volume_descriptor();
    // size_t get_path_table_count(char* dest, size_t size);
    iso_path_table_entry_header get_path_table_root_dir();
    size_t get_dir_entry_size(size_t start_lba);
    int populate_directory(ArtDirectory*& target_dir);
    ArtDirectory* make_root_directory(const iso_path_table_entry_header& path_table_root);
    int populate_directory_recursive(ArtDirectory* target_dir);
    int populate_file_tree();
    volatile bool BM_waiting_for_transfer = false;


private:
    ATAPIDrive* drive_dev;
    PCIDevice* pci_dev;
    BusMasterController* bm_dev;
    u32 current_lba = 0;
    ArtDirectory* root_directory = nullptr;
    iso_path_table_entry_header path_table_root;


};

#endif //IDE_DEVICE_H
