//
// Created by artypoole on 15/09/24.
//

#ifndef ARTFILE_H
#define ARTFILE_H

#include "ArtDirectory.h"
#include "Files.h"


class ArtDirectory;

class ArtFile {
public:
    ArtFile(ArtDirectory* parent, const FileData& data); // real file on device
    ArtFile(StorageDevice* dev, char* tmp_filename); // virtual device
    ArtFile() = default;
    ~ArtFile() = default;


    size_t read(char* dest, size_t byte_count) const;
    int seek(size_t byte_offset, int whence);
    int write(const char* src, size_t byte_count) const;
    const char* get_name();

private:
    ArtDirectory* parent_directory;
    StorageDevice* device = nullptr;
    size_t first_byte = 0;
    size_t size = 0; // bytes
    tm datetime{};
    size_t file_name_length = 0;
    // u64 permissions;
    char* filename = nullptr;

    ArtFile* next_file = nullptr;
    size_t seek_pos = 0;
};



#endif //ARTFILE_H
