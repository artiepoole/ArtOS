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

#include "ELF.h"

#include <memory.h>
#include <paging.h>
#include <PagingTableUser.h>
#include <Scheduler.h>

#include "logging.h"
#include "stdio.h"
#include "art_string.h"

ELF::ELF(ArtFile* parent_file) : file(parent_file)
{
    if (file->seek(0, SEEK_SET) != 0) return;
    if (file->read(reinterpret_cast<char*>(&elf_header), sizeof(ELF_header_t)) <= 0) return;
    if (
        elf_header.e_ident.magic != ELF_MAGIC ||
        elf_header.e_ident.ei_class != ELF_CLASS_32 ||
        elf_header.e_ident.ei_version != ELF_VERSION ||
        elf_header.e_ident.ei_OS_ABI != 0 ||
        elf_header.e_machine != ELF_ISA_x86 ||
        elf_header.e_ehsize != sizeof(ELF_header_t)
    )
    {
        return;
    }
    if (elf_header.e_ident.ei_data != ELF_LITLE_ENDIAN)
    {
        // TODO: implement
        goto err_program_read;
    }
    if (elf_header.e_phnum != 0)
    {
        const size_t n_bytes = sizeof(ELF_program_header_t) * elf_header.e_phnum;
        program_header_table = static_cast<ELF_program_header_t*>(art_alloc(n_bytes, 0));
        if (program_header_table == NULL) goto err_program_read;
        if (file->seek(elf_header.e_phoff, SEEK_SET) != elf_header.e_phoff) goto err_program_read;
        if (file->read(reinterpret_cast<char*>(program_header_table), sizeof(ELF_program_header_t)) <= 0) art_free(program_header_table);
    }
    if (elf_header.e_shnum != 0)
    {
        const size_t n_bytes = sizeof(ELF_section_header_t) * elf_header.e_shnum;
        section_header_table = static_cast<ELF_section_header_t*>(art_alloc(n_bytes, 0));
        if (section_header_table == NULL) goto err_program_read;
        if (file->seek(elf_header.e_shoff, SEEK_SET) != elf_header.e_shoff) goto err_program_read;
        if (file->read(reinterpret_cast<char*>(section_header_table), n_bytes) <= 0) goto err_section_read;;
    }
    if (section_header_table[elf_header.e_shstrndx].sh_size > 0)
    {
        const size_t string_table_offset = section_header_table[elf_header.e_shstrndx].sh_offset;
        const size_t string_table_length = section_header_table[elf_header.e_shstrndx].sh_size;
        string_table = static_cast<char*>(art_alloc(string_table_length, 0));
        if (file->seek(string_table_offset, SEEK_SET) != string_table_offset) goto err_string_table_read;
        if (file->read(string_table, string_table_length) <= 0) art_free(string_table);
    }
    return;

err_string_table_read:
    art_free(section_header_table);
err_section_read:
    art_free(program_header_table);
err_program_read:
    art_string::memset(&elf_header, 0, sizeof(ELF_header_t));
}

int ELF::execute()
{
    auto user_table = new PagingTableUser();
    // loop through section headers
    for (size_t i = 0; i < elf_header.e_shnum; i++)
    {
        const auto& header = section_header_table[i];
        if (header.sh_addr > 0 && header.sh_flags & ELF_FLAG_ALLOCATE)
        {
            size_t fid = 0;
            //TODO: mark as user access
            void* section = kmmap(0, header.sh_size, header.sh_flags & ELF_FLAG_WRITABLE, PAGING_USER, fid, 0);
            user_table->assign_page_table_entry(
                kget_mapping_target(section),
                virtual_address_t(header.sh_addr),
                header.sh_flags & ELF_FLAG_WRITABLE, TODO
            );
        }
    }
    Scheduler::execute_from_paging_table(user_table, file->get_name(), elf_header.e_entry);
    return 0;
}

bool ELF::is_executable()
{
    return elf_header.e_ident.magic == ELF_MAGIC;
}

