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


struct window_t
{
    u32 x1;
    u32 y1;
    u32 x2;
    u32 y2;
    u32 w;
    u32 h;
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
    window_t _screen_region{};

public:
    VideoGraphicsArray(const multiboot2_tag_framebuffer_common* framebuffer_info);
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
    void copy_region(const u32* src, size_t x, size_t y, size_t w, size_t h) const;
    void drawSplash() const;
    void draw_region(const u32* buffer_to_draw) const;
    [[nodiscard]] window_t* getScreen();


    // Font Definition
};

#endif
