//
// Created by artypoole on 15/09/24.
//

#ifndef STORAGEDEVICE_H
#define STORAGEDEVICE_H

#include "types.h"
class ArtFile;

class StorageDevice
{
public:
    virtual ~StorageDevice() = default;
    virtual size_t read(char* dest, size_t byte_offset, size_t byte_count) = 0;
    virtual int seek(size_t byte_offset, int whence) = 0;
    virtual size_t write(const char* src, size_t byte_offset, size_t byte_count) = 0;
    virtual int mount() =0;
    virtual ArtFile* find_file(const char* filename)=0;
    virtual size_t get_block_size() = 0;
    virtual size_t get_block_count() = 0;
    virtual size_t get_sector_size() = 0;
};


#endif //STORAGEDEVICE_H
