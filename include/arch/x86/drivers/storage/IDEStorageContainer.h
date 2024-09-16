//
// Created by artypoole on 05/09/24.
//

#ifndef IDE_DEVICE_H
#define IDE_DEVICE_H

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
    ~IDEStorageContainer() override = default; // TODO: remove PRDT?

    int read(void* dest, size_t byte_offset, size_t n_bytes) override;
    int seek(size_t offset, int whence) override {return -NOT_IMPLEMENTED;}
    int write(void* src, size_t byte_count, size_t byte_offset) override {return -NOT_IMPLEMENTED;};


    int load_file(char* filename);
    void notify() override;
    int prep_DMA_read(size_t lba, size_t n_sectors);

    void start_DMA_transfer();
    int wait_for_DMA_transfer() const;
    int stop_DMA_read();
    volatile bool BM_waiting_for_transfer = false;


private:
    ATAPIDrive* drive_dev;
    PCIDevice* pci_dev;
    BusMasterController* bm_dev;
    u32 current_lba = 0;

};

#endif //IDE_DEVICE_H
