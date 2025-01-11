//
// Created by artypoole on 20/07/24.
//

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

#ifndef FILES_H
#define FILES_H

#include "types.h"
#include "time.h"
#include "_PDCLIB_internal.h"

#ifdef __cplusplus
class ArtFile;
class StorageDevice;

using ReadFunc = size_t (char* data, size_t count);
using WriteFunc = size_t(const char* data, size_t count);
#endif

struct FileInfo
{
    char* path;
};

#ifdef __cplusplus
// TODO: should this replace FileInfo? I don't construct a path.
struct FileData
{
    StorageDevice* device = nullptr;
    size_t LBA_address = 0;
    size_t data_length_LE = 0; // bytes
    tm datetime{};
    size_t file_name_length = 0;
    // u64 permissions;
    char* filename = nullptr;
};

struct DirectoryData
{
    StorageDevice* device = nullptr;
    size_t descriptor_LBA = 0;
    size_t descriptor_length = 0; // bytes
    tm datetime = {};
    size_t dir_name_length = 0;
    // u64 permissions;
    char* directory_name = nullptr;
};


ArtFile* get_file_handle(size_t fd);
int register_file_handle(size_t fd, ArtFile* file);
void register_storage_device(StorageDevice* dev);
void deregister_storage_device(StorageDevice* dev);

extern "C" {
#endif

enum protection_flags
{
    PROT_NONE = -1,
    PROT_READ = 1 << 0,
    PROT_WRITE = 1 << 1,
    PROT_EXEC = 1 << 2,
};

enum file_open_flags
{
    O_RDONLY = 1 << 3,
    O_WRONLY = 1 << 4,
    O_RDWR = 1 << 5,
    O_CLOEXEC = 1 << 6,
    O_CREAT = 1 << 7,
    O_DIRECTORY = 1 << 8,
    O_EXCL = 1 << 9,
    O_NOCTTY = 1 << 10,
    O_NOFOLLOW = 1 << 11,
    O_TMPFILE = 1 << 12,
    O_TRUNC = 1 << 13,
};

enum mmap_flags
{
    MAP_SHARED = 1 << 0,
    MAP_SHARED_VALIDATE = 1 << 1,
    MAP_PRIVATE = 1 << 2,
};


int art_close(size_t fd);

int art_open(const char* filename, unsigned int mode);

int art_write(int fd, const char* buf, unsigned long count);

int art_read(int fd, char* buf, size_t count);

int art_exec(const int fid);

_PDCLIB_int_least64_t art_seek(const struct _PDCLIB_file_t* stream, _PDCLIB_int_least64_t offset, int whence);


#ifdef __cplusplus
}
#endif

#endif //FILES_H
