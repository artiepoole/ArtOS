//
// Created by artypoole on 15/09/24.
//
#include "ArtDirectory.h"
#include "ArtFile.h"

#include <RTC.h>
#include <stdio.h>
#include <StorageDevice.h>
#include <string.h>


ArtFile::ArtFile(ArtDirectory* parent, const FileData& data) : parent_directory(parent)
{
    device = data.device;
    first_byte = data.LBA_address * device->get_block_size();
    size = 0; // bytes
    datetime = data.datetime;
    file_name_length = data.file_name_length;
    filename = data.filename;
}

ArtFile::ArtFile(StorageDevice* dev, char* tmp_filename): device(dev)
{
    first_byte = 0;
    size = -1; // bytes
    datetime = *RTC::get().getTime();
    file_name_length = strlen(tmp_filename);
    filename = strdup(tmp_filename);
}

/* return number of bytes read or <0 = error */
size_t ArtFile::read(char* dest, const size_t byte_count) const
{
    // TODO: handle checks here.
    return device->read(dest, first_byte + seek_pos, byte_count);
}

/* return new position in bytes or <0 = error */
int ArtFile::seek(size_t byte_offset, int whence)
{
    switch (whence)
    {
    case SEEK_SET:
        {
            seek_pos = byte_offset;
            return 0;
        }
    case SEEK_CUR:
        {
            seek_pos += byte_offset;
            return 0;
        }
    case SEEK_END:
        {
            seek_pos = size - byte_offset - 1;
            return 0;
        }
    default: return -1;
    }
    // TODO: handle checks here.
    return device->seek(byte_offset + first_byte, whence);
}

/* return number of bytes written or <0 = error */
int ArtFile::write(const char* src, const size_t byte_count) const
{
    return device->write(src, seek_pos, byte_count);
}

const char* ArtFile::get_name()
{
    return filename;
}
