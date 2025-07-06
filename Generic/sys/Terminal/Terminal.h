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

#ifndef TERMINAL_H
#define TERMINAL_H

#include "ArtFile.h"
#include "StorageDevice.h"

#include "types.h"
#include "colours.h"
#include "art_string.h"

struct terminal_char_t {
    char letter;
    PALETTE_t colour;
};

/* Terminal class for interacting with text on screen.
 * If not initialised, all writes will append to a queue to be rendered upon initialisation.
 * If initialised, all writes will add text to screen.
 * log can be used to print many strings and variables to screen in one call
 * write has many type overloads for default types.
 *
 */
class Terminal {
public:
    // Single instance.
    Terminal(u32 x, u32 y, u32 width, u32 height);

    ~Terminal();

    static Terminal &get();

    static ArtFile *get_stdout_file();

    static ArtFile *get_stderr_file();

    // remove copy functionality
    Terminal(Terminal const &other) = delete;

    Terminal &operator=(Terminal const &other) = delete;

    static void newLine();

    void userLine();

    void setScale(u32 new_scale);

    void setRegion(u32 x, u32 y, u32 width, u32 height);

    static u32 getScale();

    static void clear();

    static void stop_drawing();

    static void resume_drawing();

    // for use with stdout/stderr
    static u32 user_write(const char *data, u32 count);

    static u32 user_err(const char *data, u32 count);


    // for normal use/logging.
    static u32 write(bool b);

    static u32 write(const char *data, size_t len, PALETTE_t colour = COLOR_BASE0); // buffer of fixed len
    static u32 write(const char *data, PALETTE_t colour = COLOR_BASE0); // buffer without known length also with colour
    static u32 write(char c, PALETTE_t colour = COLOR_BASE0); // single char

    static void setChar(size_t x, size_t y, char c, PALETTE_t colour);

    static void time_stamp();

    static void backspace();

    static void refresh();

private:
    static void _scroll();

    static void _draw_changes();

    static void _putChar(terminal_char_t ch, u32 origin_x, u32 origin_y);

    static void _write_to_screen(const char *data, u32 count, PALETTE_t colour); // or append to queue if not init'ed
    static void _append_to_queue(const char *data, u32 count, PALETTE_t colour);

    static void _render_queue(const terminal_char_t *data, size_t len); // used to display data from before init.
    static void _update_cursor();

public:
    template<typename int_like>
        requires is_int_like_v<int_like> && (!is_same_v<int_like, char>)
    // Any interger like number but not a char or char array.
    void write(const int_like val, const bool hex = false, PALETTE_t colour = colour_value) {
        char out_str[255];
        if (hex) {
            art_string::hex_from_int(val, out_str, sizeof(val));
        } else {
            art_string::string_from_int(val, out_str);
        }
        write(out_str, colour);
    }

    template<typename type_t>
    void log(type_t const &arg1) {
        time_stamp();
        write(arg1);
        newLine();
    }

    template<typename... args_t>
    void log(args_t &&... args) {
        time_stamp();
        (write(args), ...);
        newLine();
    }
};

class TermFileWrapper : StorageDevice {
public:
    explicit TermFileWrapper(bool stderr);


    ArtFile *get_file();

    i64 read(char *, size_t, size_t) override { return 0; }

    i64 write(const char *data, size_t, size_t byte_count) override;

    virtual i64 async_read(char* dest, size_t byte_offset, size_t byte_count) { return -1; }
    virtual bool device_busy() { return false; }
    virtual i64 async_n_read() { return -1; }
    i64 seek(u64, int) override { return 0; }
    int mount() override { return 0; }

    ArtFile *find_file([[maybe_unused]] const char *filename) override;

    char *get_name() override;

    size_t get_block_size() override { return 1; }
    size_t get_block_count() override { return -1; }
    size_t get_sector_size() override { return 1; }

private:
    bool is_stderr = false;
    char name[7];
    ArtFile file{};
};

Terminal &get_terminal();


#endif //TERMINAL_H
