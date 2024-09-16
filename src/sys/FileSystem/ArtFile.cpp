//
// Created by artypoole on 15/09/24.
//
#include "ArtDirectory.h"
#include "ArtFile.h"

#include <StorageDevice.h>




/* return number of bytes read or <0 = error */
int ArtFile::read(void* dest, size_t byte_count, size_t byte_offset)
{
    return file_data.device->read(dest, byte_count, byte_offset);
}

/* return new position in bytes or <0 = error */
int ArtFile::seek(size_t byte_offset, int whence)
{
    return file_data.device->seek(byte_offset, whence);
}

/* return number of bytes written or <0 = error */
int ArtFile::write(void* src, size_t byte_count, size_t byte_offset)
{
    return file_data.device->write(src, byte_count, byte_offset);
}

const char* ArtFile::get_name()
{
    return file_data.filename;
}
