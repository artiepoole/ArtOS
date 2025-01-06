//
// Created by artypoole on 15/09/24.
//
#include "ArtDirectory.h"

#include "StorageDevice.h"
#include "string.h"

#include "Files.h"
#include "ArtFile.h"

ArtDirectory::ArtDirectory(ArtDirectory* parent, const DirectoryData& data): parent_directory(parent)
{
    device = data.device;
    descriptor_loc_bytes = data.descriptor_LBA * device->get_block_size();
    datetime = data.datetime;
    descriptor_length = data.descriptor_length;
    dir_name_length = data.dir_name_length;
    directory_name = data.directory_name;
}

ArtDirectory::~ArtDirectory() = default; // TODO: remove this and free the dir name etc.

/* returns 0 on success, <0 on error */
int ArtDirectory::add_file(const FileData& data)
{
    files.append(ArtFile{this, data});
    return 0;
}

/* returns 0 on success, <0 on error */
int ArtDirectory::add_subdir(ArtDirectory* parent, const DirectoryData& data)
{
    directories.append(ArtDirectory{parent, data});
    return 0;
}

/* Returns pointer to file if found in this directory or nullptr */
ArtFile* ArtDirectory::search(const char* filename)
{
    return files.find_if([filename](ArtFile f) { return strcmp(f.get_name(), filename) == 0; });
}

/* Returns pointer to file if found in this directory or any of its subdirs searched recursively, or nullptr */
ArtFile* ArtDirectory::search_recurse([[maybe_unused]] const char* filename)
{
    ArtFile* res = search(filename);
    if (res != nullptr) return res;
    ArtDirectory* containing_dir = directories.find_if([filename](ArtDirectory f) { return f.search_recurse(filename); });
    if (containing_dir == nullptr) return nullptr;
    return containing_dir->search(filename);
}

ArtDirectory* ArtDirectory::get_parent()
{
    return parent_directory;
}

char* ArtDirectory::get_name()
{
    return directory_name;
}

size_t ArtDirectory::get_lba()
{
    return descriptor_loc_bytes / device->get_block_size();
}

LinkedList<ArtDirectory> const* ArtDirectory::get_dirs() const
{
    return &directories;
}

LinkedList<ArtFile> const* ArtDirectory::get_files() const
{
    return &files;
}
