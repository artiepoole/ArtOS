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
// Created by artypoole on 10/01/25.
//


#ifndef ELF_H
#define ELF_H

#include "ArtFile.h"
#include "types.h"

// https://en.wikipedia.org/wiki/Executable_and_Linkable_Format
struct eident_t
{
    u32 magic;
    char ei_class, ei_data, ei_version, ei_OS_ABI, ei_ABI_version;
    char ei_pad[7];
};

struct ELF_header_t
{
    eident_t e_ident;
    u16 e_type;
    u16 e_machine;
    u32 e_version;
    u32 e_entry;
    u32 e_phoff;
    u32 e_shoff;
    u32 e_flags;
    u16 e_ehsize;
    u16 e_phentsize;
    u16 e_phnum;
    u16 e_shentsize;
    u16 e_shnum;
    u16 e_shstrndx;
};

struct ELF_program_header_t
{
    u32 p_type;
    u32 p_offset;

    u32 p_vaddr;
    u32 p_paddr;
    u32 p_filesz;
    u32 p_memsz;
    u32 p_flags;
    u32 p_align;
};

struct ELF_section_header_t
{
    u32 sh_name;
    u32 sh_type;
    u32 sh_flags;
    u32 sh_addr;
    u32 sh_offset;
    u32 sh_size;
    u32 sh_link;
    u32 sh_info;
    u32 sh_addralign;
    u32 sh_entsize;
};

constexpr u32 ELF_MAGIC = 0x464c457f;
constexpr u32 ELF_CLASS_32 = 1;
constexpr u32 ELF_VERSION = 1;
constexpr u32 ELF_LITLE_ENDIAN = 1;
constexpr u32 ELF_BIGENDIAN = 2;
constexpr u32 ELF_ISA_x86 = 0x3;
constexpr u32 ELF_FLAG_WRITABLE = 0x1;
constexpr u32 ELF_FLAG_ALLOCATE = 0x2;
constexpr u32 ELF_FLAG_EXECUTABLE = 0x4;

class ELF
{
public:
    ELF(ArtFile* parent_file);
    int execute();
    bool is_executable();

private:
    void mmap();
    ArtFile* file;
    ELF_header_t elf_header;
    ELF_program_header_t* program_header_table;
    ELF_section_header_t* section_header_table;
    char* string_table;
};


#endif //ELF_H
