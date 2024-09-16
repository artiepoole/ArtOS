//
// Created by artypoole on 20/07/24.
//

#ifndef FILES_H
#define FILES_H

class StorageDevice;

#include "time.h"

#include "types.h"


typedef u32 (ReadFunc)(char* dest, u32 count);
typedef u32 (WriteFunc)(const char* data, u32 count);

struct FileInfo
{
    char* path;
};

struct FileHandle
{
    FileInfo info;
    ReadFunc* read;
    WriteFunc* write;
};

// TODO: should this replace FileInfo? I don't construct a path.
struct FileData{
    StorageDevice* device = nullptr;
    u32 LBA_address = 0;
    u32 data_length_LE = 0; // bytes
    tm datetime{};
    size_t file_name_length = 0;
    // u64 permissions;
    char* filename = nullptr;
};

struct DirectoryData{
    StorageDevice* device = nullptr;
    u32 descriptor_LBA = 0;
    u32 descriptor_length = 0; // bytes
    tm datetime = {};
    size_t dir_name_length = 0;
    // u64 permissions;
    char* directory_name = nullptr;
};


FileHandle* get_file_handle(int fd);
u32 register_file_handle(int fd, const char* path, ReadFunc* read, WriteFunc* write);
void close_file_handle(int fd);
extern "C"
int open(const char* filename, unsigned int mode);
extern "C"
int write(int fd, const char* buf, unsigned long count);
extern "C"
int read( int fd, char* buf, const size_t count);

#endif //FILES_H
