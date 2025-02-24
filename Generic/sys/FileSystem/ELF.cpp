//
// Created by artypoole on 10/01/25.
//

#include "ELF.h"

#include "logging.h"
#include "stdio.h"
#include "stdlib.h"
#include "string.h"

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
        program_header_table = static_cast<ELF_program_header_t*>(malloc(n_bytes));
        if (program_header_table == NULL) goto err_program_read;
        if (file->seek(elf_header.e_phoff, SEEK_SET) != elf_header.e_phoff) goto err_program_read;
        if (file->read(reinterpret_cast<char*>(program_header_table), sizeof(ELF_program_header_t)) <= 0) free(program_header_table);
    }
    if (elf_header.e_shnum != 0)
    {
        const size_t n_bytes = sizeof(ELF_section_header_t) * elf_header.e_shnum;
        section_header_table = static_cast<ELF_section_header_t*>(malloc(n_bytes));
        if (section_header_table == NULL) goto err_program_read;
        if (file->seek(elf_header.e_shoff, SEEK_SET) != elf_header.e_shoff) goto err_program_read;
        if (file->read(reinterpret_cast<char*>(section_header_table), n_bytes) <= 0) goto err_section_read;;
    }
    if (section_header_table[elf_header.e_shstrndx].sh_size > 0)
    {
        const size_t string_table_offset = section_header_table[elf_header.e_shstrndx].sh_offset;
        const size_t string_table_length = section_header_table[elf_header.e_shstrndx].sh_size;
        string_table = static_cast<char*>(malloc(string_table_length));
        if (file->seek(string_table_offset, SEEK_SET) != string_table_offset) goto err_string_table_read;
        if (file->read(string_table, string_table_length) <= 0) free(string_table);
        LOG("String table: ", string_table);
    }

    // TODO: find the symbol table?
    // I can now use the section_header_table[idx].sh_name as an index into the string table to figure out what the section is.
    // We don't need the entire file to be mapped into memory. Only the relevant sections do: the ones with valid address values.
    // I can loop through all and keep track of the start address and end and figure out the max_address required and then map this
    // on to the end of the stack or something when executing. E.g. my hello.elf starts at 0x100000 and the actual code and data sections are only


    // in the section table, the address is the destination address and the offset is the loation in the file. The size is the n_bytes in both caes.
    return;

err_string_table_read:
    free(section_header_table);
err_section_read:
    free(program_header_table);
err_program_read:
    memset(&elf_header, 0, sizeof(ELF_header_t));
}

int ELF::execute()
{
    //TODO: do checks on page alignment to ensure that allocated memory is read only
    // in one page and writeable in a new page.
    mmap();
    // TODO: syscall to load this function.
    return -1;
}

bool ELF::is_executable()
{
    return elf_header.e_ident.magic == ELF_MAGIC;
}

void ELF::mmap()
{
    // TODO: map to memory
}
