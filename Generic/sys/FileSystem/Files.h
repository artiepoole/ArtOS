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


/* TODO
 * The following syscalls should be implemented in this form.
 * Currently only mount exists and is does not mount to a dir, nor does ArtOS have a concept of a path right now.
 *
 *  mount/unmount: mount a device to a directory
 *  open/close: create and release a file descriptor for a given file
 *  read/write: read or write data from a file descriptor
 *  mkdir/rmdir: create and delete directories
 *  link/unlink: create and delete hard links
 *  raname: move files and directories
 *  chmod/chown: change permissions and ownership on files
 *  stat: gets information about a file
 *  getdents: gets directory entries
 *  sync: flush filesystem buffers (should be done by close)
 *  mknod: creates a filesystem node (file, device, special file, or named pipe)
 *
 */


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
extern "C"
_PDCLIB_int_least64_t seek(const _PDCLIB_file_t* stream, _PDCLIB_int_least64_t offset, int whence) ;

void register_storage_device(StorageDevice* dev);
void deregister_storage_device(StorageDevice* dev);


#endif //FILES_H
