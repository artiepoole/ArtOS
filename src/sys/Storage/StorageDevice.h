//
// Created by artypoole on 15/09/24.
//

#ifndef STORAGEDEVICE_H
#define STORAGEDEVICE_H

#include "types.h"

class StorageDevice
{
public:
    virtual ~StorageDevice() = default;
    virtual int read(void* dest, size_t byte_offset, size_t byte_count) = 0;
    virtual int seek(size_t byte_offset, int whence) = 0;
    virtual int write(void* src, size_t byte_offset, size_t byte_count) =0;
};


#endif //STORAGEDEVICE_H
