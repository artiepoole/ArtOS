//
// Created by artypoole on 15/09/24.
//
#include "ArtDirectory.h"

#include <string.h>

#include "ArtFile.h"

ArtDirectory::ArtDirectory(ArtDirectory* parent, const DirectoryData& data): parent_directory(parent), directory_data(data)
{
}

ArtDirectory::~ArtDirectory() = default; // TODO: remove this and free the dir name etc.

/* returns 0 on success, <0 on error */
int ArtDirectory::add_file(const FileData& data)
{
    files.append(ArtFile{data});
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
    return files.find_if([filename](ArtFile f) { return f.get_name() == filename; });
}

/* Returns pointer to file if found in this directory or any of its subdirs searched recursively, or nullptr */
ArtFile* ArtDirectory::search_recurse(char* filename)
{
    // TODO: implement the recusrive search.
    return nullptr;
}

ArtDirectory* ArtDirectory::get_parent()
{
    return parent_directory;
}

char* ArtDirectory::get_name()
{
    return directory_data.directory_name;
}
