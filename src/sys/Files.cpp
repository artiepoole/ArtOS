//
// Created by artypoole on 20/07/24.
//

#include "Files.h"

#include <Serial.h>
#include <stdio.h>

#include "string.h"
#include "stdlib.h"

#define MAX_HANDLES 500

#define ERR_TOO_MANY_FILES -1
#define ERR_HANDLE_TAKEN -2
#define ERR_NOT_FOUND -3

static struct FileHandle* handles[MAX_HANDLES] = {0};

struct FileHandle* get_file_handle(int fd)
{
    return handles[fd];
}

int find_free_handle()
{
    for (int i = 0; i < MAX_HANDLES; i++)
    {
        if (handles[i] == NULL)
        {
            return i;
        }
    }
    return ERR_TOO_MANY_FILES;
}

u32 register_file_handle(const int fd, const char* path, ReadFunc* read, WriteFunc* write)
{
    if (handles[fd] != NULL)
    {
        return ERR_HANDLE_TAKEN;
    }

    handles[fd] = static_cast<FileHandle*>(malloc(sizeof(FileHandle)));
    handles[fd]->read = read;
    handles[fd]->write = write;

    handles[fd]->info.path = static_cast<char*>(malloc(sizeof(char) * strlen(path) + 1));
    strcpy(handles[fd]->info.path, path);
    return 0;
}

void close_file_handle(int fd)
{
    free(handles[fd]);
    handles[fd] = NULL;
}

extern "C"
int write(const int fd, const char* buf, const unsigned long count)
{
    const FileHandle* h = get_file_handle(fd);
    if (h == NULL)
    {
        // unknown FD
        return -1;
    }
    return h->write(buf, count);
}

extern "C"
int read(const int fd, char* buf, const size_t count)
{
    const FileHandle* h = get_file_handle(fd);
    if (h == NULL)
    {
        // unknown FD
        return -1;
    }
    return h->read(buf, count);
}

u32 doomwad_seek_pos = 0;
extern u32 doom_wad_loc_ptr;
extern u32 doom_wad_end_ptr;
auto doom_wad_loc = reinterpret_cast<char*>(&doom_wad_loc_ptr);
auto doom_wad_end = reinterpret_cast<char*>(&doom_wad_end_ptr);
u32 doomwad_size = reinterpret_cast<u32>(doom_wad_end) - reinterpret_cast<u32>(doom_wad_loc);

extern "C"
u32 doom_seek(u32 offset, int whence)
{
    if (offset > doomwad_size)
    {
        return -1;
    }

    switch (whence)
    {
    case SEEK_SET:
        {
            doomwad_seek_pos = offset;
            return 0;
        }
    case SEEK_CUR:
        {
            doomwad_seek_pos += offset;
            return 9;
        }
    case SEEK_END:
        {
            doomwad_seek_pos += doomwad_size - offset - 1;
            return 0;
        }
    default: return -1;
    }
}


extern "C"
u32 doomwad_read(char* dest, u32 count)
{
    size_t i = 0;
    while (i < doomwad_size && i < count + doomwad_seek_pos)
    {
        dest[i] = doom_wad_loc[i + doomwad_seek_pos];
        i++;
    }
    return i;
}

extern "C"
u32 doomwad_write(const char* data, u32 count)
{
    return -1;
    // Should not do this
}


extern "C"
int open(const char* filename, unsigned int mode)
{
    // TODO this is a stub. We probably want some kind of dispatch to filesystems/mounts so we can mount com0 to
    //  be opened here.

    if (strcmp("/dev/com1", filename) == 0)
    {
        int fd = find_free_handle();
        int err = register_file_handle(fd, filename, Serial::com_read, Serial::com_write);
        if (err != 0)
        {
            return err;
        }
        return fd;
    }

    if (strcmp("doomw1.wad", filename) == 0)
    {
        int fd = find_free_handle();
        int err = register_file_handle(fd, filename, doomwad_read, doomwad_write);
        if (err != 0)
        {
            return err;
        }
        return fd;
    }
    return ERR_NOT_FOUND;
}
