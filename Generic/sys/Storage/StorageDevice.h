// ArtOS - hobby operating system by Artie Poole
// Copyright (C) 2025 Stuart Forbes Poole <artiepoole>
//
//     This program is free software: you can redistribute it and/or modify
//     it under the terms of the GNU General Public License as published by
//     the Free Software Foundation, either version 3 of the License, or
//     (at your option) any later version.
//
//     This program is distributed in the hope that it will be useful,
//     but WITHOUT ANY WARRANTY; without even the implied warranty of
//     MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//     GNU General Public License for more details.
//
//     You should have received a copy of the GNU General Public License
//     along with this program.  If not, see <https://www.gnu.org/licenses/>

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
    virtual i64 read(char* dest, size_t byte_offset, size_t byte_count) = 0;
    virtual i64 seek(u64 byte_offset, int whence) = 0;
    virtual i64 write(const char* src, size_t byte_offset, size_t byte_count) = 0;
    virtual int mount() =0;
    virtual ArtFile* find_file(const char* filename) =0;
    virtual size_t get_block_size() = 0;
    virtual size_t get_block_count() = 0;
    virtual size_t get_sector_size() = 0;
    virtual char* get_name() = 0;
};


#endif //STORAGEDEVICE_H
