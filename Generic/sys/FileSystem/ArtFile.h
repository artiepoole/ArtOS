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

#ifndef ARTFILE_H
#define ARTFILE_H

#include "time.h"
#include "types.h"

class ArtDirectory;
class StorageDevice;

struct FileData;

class ArtFile {
public:
    ArtFile(ArtDirectory* parent, const FileData& data); // real file on device
    ArtFile(StorageDevice* dev, char* tmp_filename); // virtual device
    ArtFile() = default;
    ~ArtFile() = default;


    size_t read(char* dest, size_t byte_count);
    int start_async_read(char* dest, size_t byte_count) const;
    bool device_busy() const;
    _PDCLIB_int_least64_t seek(u64 byte_offset, int whence);
    int write(const char* src, size_t byte_count);
    const char* get_name();
    int get_parent_path(char* dest);
    int get_abs_path(char* dest);

    i64 async_n_read();

private:
    ArtDirectory* parent_directory;
    StorageDevice* device = nullptr;
    u64 first_byte = 0;
    u64 size = 0; // bytes
    tm datetime;
    size_t file_name_length = 0;
    // u64 permissions;
    char* filename = nullptr;

    ArtFile* next_file = nullptr;
    u64 seek_pos = 0;
};



#endif //ARTFILE_H
