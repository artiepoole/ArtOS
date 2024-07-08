/*

	This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <https://www.gnu.org/licenses/>.
*/

#ifndef __MYOS__DRIVERS__VGA_H
#define __MYOS__DRIVERS__VGA_H

#include "multiboot2.h"
#include "types.h"
#include "splash_screen.h"
#include "string.h"
#include "serial.h"
#include "font.h"

// Solarised colours definitions
enum COLORS
{
    COLOR_BASE03 = 0x002b36, // Darkest (near black, bluey grey)
    COLOR_BASE02 = 0x073642,
    COLOR_BASE01 = 0x586e75,
    COLOR_BASE00 = 0x657b83,
    COLOR_BASE0 = 0x839496,
    COLOR_BASE1 = 0x93a1a1,
    COLOR_BASE2 = 0xeee8d5,
    COLOR_BASE3 = 0xfdf6e3, // Lightest (cream)
    COLOR_YELLOW = 0xb58900,
    COLOR_ORANGE = 0xcb4b16,
    COLOR_RED = 0xdc322f,
    COLOR_MAGENTA = 0xd33682,
    COLOR_VIOLET = 0x6c71c4,
    COLOR_BLUE = 0x268bd2,
    COLOR_CYAN = 0x2aa198,
    COLOR_GREEN = 0x859900,
};

class VideoGraphicsArray
{
protected:


public:
    u32 width;
    u32 height;

protected:
    u32* _buffer;
    u32* _screen;

    void _scrollTerminal() const;
    void _renderTerminal() const;


    /*
     * 8 * 8 font 1 bit per pixel, 64 bits per charactor
     * - I used fonted in TempleOS to draw it.
    */
public:
    VideoGraphicsArray(const multiboot_header* boot_header, u32* _buffer);
    ~VideoGraphicsArray();
    static VideoGraphicsArray& get();

    void putPixel(u32 x, u32 y, u32 color) const;
    void putChar(char ch, u32 x, u32 y, u32 color) const;
    void putStr(const char* ch, u32 x, u32 y, u32 colorIndex) const;
    void fillRectangle(u32 x, u32 y, u32 w, u32 h, u32 color) const;
    void bufferToScreen(bool clear) const;
    void clearBuffer() const;
    void drawSplash() const;
    void setScale(u8 new_scale) const;
    static u8 getScale();
    void clearWindow() const;
    void writeString(const char* data) const;
    void writeChar(char data) const;

    template <typename int_like>
    void writeInt(int_like val)
    {
        char out_str[255]; // long enough for any int type possible
        const size_t len = string_from_int(val, out_str);
        char trimmed_str[len];
        for (size_t j = 0; j <= len; j++)
        {
            trimmed_str[j] = out_str[j];
        }
        writeString(trimmed_str);
        //log.write_int(val);
        // log.new_line();
    }

    template <typename int_like1>
    void writeHex(int_like1 val)
    {
        // u16 n_bytes = log2(val);
        char out_str[255];
        const size_t len = hex_from_int(val, out_str, sizeof(val));
        char trimmed_str[len];
        for (size_t j = 0; j < len; j++)
        {
            trimmed_str[j] = out_str[j];
        }
        writeString(trimmed_str);
    }


    // Font Definition
};

#endif
