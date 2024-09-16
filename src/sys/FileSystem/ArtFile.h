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
    explicit ArtFile(const FileData& data) : file_data(data){};
    ~ArtFile() = default;


    int read(void* dest, size_t byte_count, size_t byte_offset);
    int seek(size_t byte_offset, int whence);
    int write(void* src, size_t byte_count, size_t byte_offset);
    const char* get_name();

private:
    FileData file_data;
    ArtFile* next_file = nullptr;
};



#endif //ARTFILE_H
