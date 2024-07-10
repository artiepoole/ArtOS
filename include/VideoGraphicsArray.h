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

#include "multiboot_header.h"
#include "types.h"
#include "splash_screen.h"
#include "string.h"
#include "Serial.h"
#include "FONT.h"

struct window_t
{
    u32 x1;
    u32 y1;
    u32 x2;
    u32 y2;
    u32 w;
    u32 h;
};

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
public:
    u32 width;
    u32 height;

protected:
    u32* _buffer;
    u32* _screen;

private:
    window_t _window{};
    window_t _screen_region{};

public:
    VideoGraphicsArray(const multiboot_header* boot_header, u32* buffer);
    ~VideoGraphicsArray();
    static VideoGraphicsArray& get();

    // remove copy functionality
    VideoGraphicsArray(VideoGraphicsArray const& other) = delete;
    VideoGraphicsArray& operator=(VideoGraphicsArray const& other) = delete;

    void putPixel(u32 x, u32 y, u32 color) const;

    void fillRectangle(u32 x, u32 y, u32 w, u32 h, u32 color) const;
    void draw() const;
    void clearWindow() const;
    void clearBuffer() const;
    void drawSplash() const;
    void draw_region(const u32* buffer_to_draw) const;
    [[nodiscard]] window_t * getScreen();


    // Font Definition
};

#endif
