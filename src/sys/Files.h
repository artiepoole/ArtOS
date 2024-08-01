//
// Created by artypoole on 20/07/24.
//

#ifndef FILES_H
#define FILES_H
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

FileHandle* get_file_handle(int fd);
u32 register_file_handle(int fd, const char* path, ReadFunc* read, WriteFunc* write);
void close_file_handle(int fd);
extern "C"
int open(const char* filename, unsigned int mode);
extern "C"
int write(int fd, const char* buf, unsigned long count);


#endif //FILES_H
