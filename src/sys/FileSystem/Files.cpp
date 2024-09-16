//
// Created by artypoole on 20/07/24.
//

#include "Files.h"
#include "Serial.h"
#include "logging.h"
#include <stdio.h>

#include "string.h"
#include "stdlib.h"

constexpr size_t MAX_HANDLES = 500;

constexpr int ERR_TOO_MANY_FILES = -1;
constexpr int ERR_HANDLE_TAKEN = -2;
constexpr int ERR_NOT_FOUND = -3;

// TODO: replace with linked list?
static FileHandle* handles[MAX_HANDLES] = {nullptr};

FileHandle* get_file_handle(int fd)
{
    return handles[fd];
}

int find_free_handle()
{
    for (size_t i = 0; i < MAX_HANDLES; i++)
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

    handles[fd] = new FileHandle;
    handles[fd]->read = read;
    handles[fd]->write = write;

    // TODO: use string functions here
    handles[fd]->info.path = static_cast<char*>(malloc(sizeof(char) * strlen(path) + 1));
    strcpy(handles[fd]->info.path, path);
    return 0;
}

void close_file_handle(int fd)
{
    delete handles[fd];
    handles[fd] = NULL;
}

u32 doomwad_seek_pos;
extern char doom_wad_file[];
extern char doom_wad_file_end[];
u32 doomwad_size;

extern "C"
u32 doom_seek(_PDCLIB_file_t* stream, u32 offset, int whence)
{
    if (offset > doomwad_size)
    {
        return EOF;
    }

    switch (whence)
    {
    case SEEK_SET:
        {
            doomwad_seek_pos = offset;
            stream->pos.offset = offset;
            return 0;
        }
    case SEEK_CUR:
        {
            doomwad_seek_pos += offset;
            stream->pos.offset += offset;
            return 9;
        }
    case SEEK_END:
        {
            doomwad_seek_pos = doomwad_size - offset - 1;
            stream->pos.offset = doomwad_size - offset - 1;
            return 0;
        }
    default: return -1;
    }
}


extern "C"
u32 doomwad_read(char* dest, u32 count)
{
    size_t i = 0;
    while (i < doomwad_size && i < count)
    {
        dest[i] = doom_wad_file[i + doomwad_seek_pos];
        i++;
    }
    doomwad_seek_pos += i;
    return i;
}




extern "C"
int open(const char* filename, [[maybe_unused]] unsigned int mode)
{
    // TODO this is a stub. We probably want some kind of dispatch to filesystems/mounts so we can mount com0 to
    //  be opened here.

    if (strcmp("/dev/com1", filename) == 0)
    {
        int fd = find_free_handle();
        if (int err = register_file_handle(fd, filename, Serial::com_read, Serial::com_write); err != 0)
        {
            return err;
        }
        return fd;
    }

    if (strcmp("doom1.wad", filename) == 0)
    {
        doomwad_size = reinterpret_cast<u32>(&doom_wad_file_end) - reinterpret_cast<u32>(&doom_wad_file);
        TIMESTAMP();
        WRITE("doomwad loc: ");
        WRITE(reinterpret_cast<u32>(&doom_wad_file), true);
        WRITE(" doomwad end: ");
        WRITE(reinterpret_cast<u32>(&doom_wad_file_end), true);
        WRITE(" doomwad len: ");
        WRITE(doomwad_size, true);
        NEWLINE();
        int fd = find_free_handle();
        if (int err = register_file_handle(fd, filename, doomwad_read, NULL); err != 0)
        {
            return err;
        }
        return fd;
    }
    return ERR_NOT_FOUND;
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

extern "C"
int seek(struct _PDCLIB_file_t* stream, _PDCLIB_int_least64_t offset, int whence)
{
    if (strcmp(handles[stream->handle]->info.path, "doom1.wad") == 0)
    {
        return doom_seek(stream, offset, whence);
    }
    return ERR_NOT_FOUND;
}
