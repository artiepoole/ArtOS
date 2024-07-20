//
// Created by artypoole on 20/07/24.
//

#ifndef FILES_H
#define FILES_H


typedef int (ReadFunc)(char* dest, unsigned long count);
typedef int (WriteFunc)(const char* data, unsigned long count);

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
int register_file_handle(int fd, const char* path, ReadFunc* read, WriteFunc* write);
void close_file_handle(int fd);
int open(const char* filename, unsigned int mode);
int write(int fd, const char* buf, unsigned long count);


#endif //FILES_H
