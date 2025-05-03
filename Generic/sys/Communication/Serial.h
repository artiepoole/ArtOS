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
// Created by artypoole on 25/06/24.
//
#ifndef SERIAL_H
#define SERIAL_H

#include "types.h"
#include <StorageDevice.h>
#include "art_string.h"


class ArtFile;

#define PORT 0x3f8          // COM1

class Serial : StorageDevice
{
private:
    static char _read_one_byte();
    static void _send_one_byte(unsigned char a);
    static int _get_read_ready_status();
    static int _get_transmit_empty();
    static void _write_buffer(const char* data, size_t size);
    char name[11] = "/dev/com1";

public:
    Serial();
    void link_file();
    // ~Serial();

    // static Serial& get();
    static ArtFile*& get_file();

    // remove copy functionality
    Serial(Serial const& other) = delete;
    Serial& operator=(Serial const& other) = delete;

    bool connected;
    void register_device();
    void newLine();
    void write(bool b);
    // void write(unsigned char c);
    void write(char c);
    void write(const char* data);
    void write(const char* data, size_t len);
    static u32 com_read(char* dest, u32 count);
    static u32 com_write(const char* data, u32 count);

    i64 read(char* dest, [[maybe_unused]] size_t byte_offset, size_t byte_count) override { return com_read(dest, byte_count); }
    i64 write(const char* data, [[maybe_unused]] size_t byte_offset, size_t byte_count) override { return com_write(data, byte_count); }
    i64 seek([[maybe_unused]] u64 byte_offset, [[maybe_unused]] int whence) override { return 0; }
    char* get_name() override { return name; }


    int mount() { return 0; }

    ArtFile* find_file([[maybe_unused]] const char* filename) override
    {
        if (art_string::strcmp(filename, "/dev/com1") == 0) return get_file();
        else return nullptr;
    }

    size_t get_block_size() override { return 1; }

    size_t get_block_count() override { return -1; }

    size_t get_sector_size() override { return 1; }

    void time_stamp();

    template <typename int_like>
        requires is_int_like_v<int_like> && (!is_same_v<int_like, char>) // Any interger like number but not a char or char array.
    void write(const int_like val, const bool hex = false)
    {
        char out_str[255];
        if (hex)
        {
            art_string::hex_from_int(val, out_str, sizeof(val));
        }
        else
        {
            art_string::string_from_int(val, out_str);
        }
        write(out_str);
    }

    template <typename type_t>
    void log(type_t const& arg1)
    {
        time_stamp();
        write(arg1);
        newLine();
    }

    template <typename... args_t>
    void log(args_t&&... args)
    {
        time_stamp();
        (write(args), ...);
        newLine();
    }


    template <typename int_like>
        requires is_int_like_v<int_like> && (!is_same_v<int_like, char>)
    void logHex(int_like val, const char* val_name = "")
    {
        if (art_string::strlen(val_name) > 0)
        {
            write(val_name);
            write(": ");
        }
        write(val, true);
        newLine();
    };
};

Serial& get_serial();

#endif //SERIAL_H
