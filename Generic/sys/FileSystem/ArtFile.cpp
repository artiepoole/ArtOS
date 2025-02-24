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

#include "ArtDirectory.h"
#include "ArtFile.h"
#include "Files.h"
#include <RTC.h>
#include <stdio.h>
#include <StorageDevice.h>
#include <string.h>


ArtFile::ArtFile(ArtDirectory* parent, const FileData& data) : parent_directory(parent)
{
    device = data.device;
    first_byte = data.LBA_address * device->get_block_size();
    size = data.data_length_LE; // bytes
    datetime = data.datetime;
    file_name_length = data.file_name_length;
    filename = data.filename;
}

ArtFile::ArtFile(StorageDevice* dev, char* tmp_filename): device(dev)
{
    first_byte = 0;
    size = -1; // bytes
    RTC::get().getTime(&datetime);
    file_name_length = strlen(tmp_filename);
    filename = strdup(tmp_filename);
}

/* return number of bytes read or <0 = error */
size_t ArtFile::read(char* dest, size_t byte_count)
{
    // TODO: handle checks here.
    // if (byte_count > 1024*64) {byte_count = 1024*64;}
    if (seek_pos + byte_count > size) { byte_count = size - seek_pos; }
    if (byte_count == 0) { return 0; }
    // TODO: figure out what this should return.
    size_t rc = device->read(dest, first_byte + seek_pos, byte_count);
    seek_pos += byte_count;
    return rc;
}

/* return new position in bytes or <0 = error */
_PDCLIB_int_least64_t ArtFile::seek(const u64 byte_offset, const int whence)
{
    if (byte_offset > size - 1) return EOF;
    switch (whence)
    {
    case SEEK_SET:
        {
            seek_pos = byte_offset;
            break;
        }
    case SEEK_CUR:
        {
            seek_pos += byte_offset;
            break;
        }
    case SEEK_END:
        {
            seek_pos = size - byte_offset - 1;
            break;
        }
    default: return -1;
    }
    return seek_pos;
    // TODO: handle checks here.
}

/* return number of bytes written or <0 = error */
int ArtFile::write(const char* src, const size_t byte_count)
{
    return device->write(src, seek_pos, byte_count);
}

const char* ArtFile::get_name()
{
    return filename;
}
