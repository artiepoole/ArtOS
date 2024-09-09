//
// Created by artypoole on 05/09/24.
//

#ifndef IDE_DEVICE_H
#define IDE_DEVICE_H

#include <ATAPIDrive.h>

#include "types.h"

#include "BusMasterController.h"
#include "PCIDevice.h"
#include "IDE_notifiable.h"

typedef int (*readFunc)();
typedef int (*seekFunc)();
typedef int (*writeFunc)();

class IDEStorageContainer: public IDE_notifiable
{
public:
    IDEStorageContainer(ATAPIDrive* drive, PCIDevice* pci_dev, BusMasterController* bm_dev);
    ~IDEStorageContainer() override = default;
    int read(u8* dest, u32 n_bytes);
    // void write(u8* src, u32 n_bytes);
    // int go_to_LBA(u32 LBA);
    u32 current_lba = 0;
    int load_file(char* filename);
    void notify() override;
    int prep_DMA_read(size_t n_sectors);
    // int prep_DMA_write();
    void start_DMA_transfer();
    int wait_for_DMA_transfer() const;
    int stop_DMA_read();
    volatile bool BM_waiting_for_transfer = false;


private:
    ATAPIDrive* drive_dev;
    PCIDevice* pci_dev;
    BusMasterController* bm_dev;

};

#endif //IDE_DEVICE_H
