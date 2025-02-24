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
// Created by artypoole on 15/09/24.
//

#ifndef ARTDIRECTORY_H
#define ARTDIRECTORY_H


#include "StorageDevice.h"
#include "LinkedList.h"
#include "time.h"

class ArtFile;
struct DirectoryData;
struct FileData;

class ArtDirectory
{
public:
    ArtDirectory(ArtDirectory* parent, const DirectoryData& data);
    ~ArtDirectory();


    int add_file(const FileData& data) ;
    int add_subdir(ArtDirectory* parent, const DirectoryData& data) ;

    //todo: remove dir and remove file

    ArtFile* search(const char* filename);
    ArtFile* search_recurse(const char* filename);
    ArtDirectory* get_parent();
    char* get_name();
    size_t get_lba();
    LinkedList<ArtDirectory> const *get_dirs() const;
    LinkedList<ArtFile> const *get_files() const;


private:

    ArtDirectory* parent_directory;
    StorageDevice* device = nullptr;
    size_t descriptor_loc_bytes = 0;
    size_t descriptor_length = 0; // bytes
    tm datetime = {};
    size_t dir_name_length = 0;
    // u64 permissions;
    char* directory_name = nullptr;
    LinkedList<ArtDirectory> directories={};
    LinkedList<ArtFile> files = {};
};




#endif //ARTDIRECTORY_H
