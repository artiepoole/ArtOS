// ArtOS - hobby operating system by Artie Poole
// Copyright (C) 2025 Stuart Forbes Poole <artiepoole>
//
//     This program is free software: you can redistribute it and/or modify
//     it under the terms of the GNU General Public License as published by
//     the Free Software Foundation, either version 3 of the License, or
//     (at your option) any later version.
//
//     This program is distributed in the hope that it will be useful,
//     but WITHOUT ANY WARRANTY; without even the implied warranty of
//     MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//     GNU General Public License for more details.
//
//     You should have received a copy of the GNU General Public License
//     along with this program.  If not, see <https://www.gnu.org/licenses/>

//
// Created by artypoole on 20/07/24.
//

#include "Files.h"

//
#include "ArtFile.h"
#include "LinkedList.h"
//
#include "Serial.h"
#include "logging.h"
#include <stdio.h>

#include "ELF.h"
#include "StorageDevice.h"

#include "stdlib.h"


constexpr size_t MAX_HANDLES = 500;

constexpr int ERR_TOO_MANY_FILES = -1;
constexpr int ERR_HANDLE_TAKEN = -2;
constexpr int ERR_NOT_FOUND = -3;

// TODO: replace with linked list?
static ArtFile* handles[MAX_HANDLES] = {nullptr};

LinkedList<StorageDevice*> devices;

void register_storage_device(StorageDevice* dev)
{
    LOG("Storage device registered: ", dev->get_name());
    devices.append(dev);
}

void deregister_storage_device(StorageDevice* dev)
{
    devices.remove(dev);
}

ArtFile* get_file_handle(int fd)
{
    return handles[fd];
}

int find_free_handle()
{
    // skip stderr, stdin, stdout
    for (size_t i = 3; i < MAX_HANDLES; i++)
    {
        if (handles[i] == NULL)
        {
            return i;
        }
    }
    return ERR_TOO_MANY_FILES;
}


void close_file_handle(int fd)
{
    delete handles[fd];
    handles[fd] = NULL;
}

int register_file_handle(size_t file_id, ArtFile* file)
{
    if (handles[file_id] != NULL)
    {
        return ERR_HANDLE_TAKEN;
    }

    handles[file_id] = file;
    return 0;
}

extern "C"
int art_open(const char* filename, [[maybe_unused]] unsigned int mode)
{
    // TODO this is unfinished. This should take a path or a working dir

    if (art_string::strcmp("/dev/com1", filename) == 0)
    {
        int fd = find_free_handle();
        if (int err = register_file_handle(fd, Serial::get_file()); err != 0)
        {
            return err;
        }
        return fd;
    }

    if (auto* file = devices.find_first<ArtFile*>([filename](StorageDevice* dev) { return dev->find_file(filename); }))
    {
        int file_id = find_free_handle();
        if (const int err = register_file_handle(file_id, file); err != 0)
        {
            return err;
        }
        return file_id;
    }

    return ERR_NOT_FOUND;
}

extern "C"
int art_close(size_t file_id)
{
    if (handles[file_id] != NULL)
    {
        return 0;
    }
    return ERR_NOT_FOUND;
}


extern "C"
int art_write(const int fd, const char* buf, const unsigned long count)
{
    ArtFile* h = get_file_handle(fd);
    if (h == NULL)
    {
        // unknown FD
        return -1;
    }
    return h->write(buf, count);
}

extern "C"
int art_read(const int file_id, char* buf, const size_t count)
{
    ArtFile* h = get_file_handle(file_id);
    if (h == NULL)
    {
        // unknown FD
        return -1;
    }
    return h->read(buf, count);
}


int art_exec(const int fid)
{
    ArtFile* h = get_file_handle(fid);
    if (auto executable = ELF(h); executable.is_executable())
    {
        return executable.execute(); // ideally this would return the exit code.
    }
    return -1; // could not execute. Should replace with real error number.
}

extern "C"
_PDCLIB_int_least64_t art_seek(const _PDCLIB_file_t* stream, _PDCLIB_int_least64_t offset, const int whence)
{
    //TODO: implement device seek_pos changes.
    if (ArtFile* h = handles[stream->handle])
    {
        return h->seek(offset, whence);
    }
    // if (strcmp(->get_name(), "doom1.wad") == 0)
    // {
    //     return doom_seek(stream, offset, whence);
    // }
    return ERR_NOT_FOUND;
}
