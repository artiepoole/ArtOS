//
// Created by artypoole on 20/07/24.
//

#ifndef FILES_H
#define FILES_H

class ArtFile;
class StorageDevice;

#include "time.h"

#include "types.h"


using ReadFunc = size_t (char* data, size_t count);
using WriteFunc = size_t(const char* data, size_t count);

struct FileInfo
{
    char* path;
};



// TODO: should this replace FileInfo? I don't construct a path.
struct FileData{
    StorageDevice* device = nullptr;
    size_t LBA_address = 0;
    size_t data_length_LE = 0; // bytes
    tm datetime{};
    size_t file_name_length = 0;
    // u64 permissions;
    char* filename = nullptr;
};

struct DirectoryData{
    StorageDevice* device = nullptr;
    size_t descriptor_LBA = 0;
    size_t descriptor_length = 0; // bytes
    tm datetime = {};
    size_t dir_name_length = 0;
    // u64 permissions;
    char* directory_name = nullptr;
};


ArtFile* get_file_handle(size_t fd);
int register_file_handle(size_t fd,  ArtFile  * file);

void close_file_handle(size_t fd);
extern "C"
int open(const char* filename, unsigned int mode);
extern "C"
size_t write(int fd, const char* buf, unsigned long count);
extern "C"
size_t read( int fd, char* buf, size_t count);

void register_storage_device(StorageDevice* dev);
void deregister_storage_device(StorageDevice* dev);


#endif //FILES_H
