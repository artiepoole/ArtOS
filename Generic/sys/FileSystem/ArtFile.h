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
    _PDCLIB_int_least64_t seek(u64 byte_offset, int whence);
    int write(const char* src, size_t byte_count);
    const char* get_name();

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
