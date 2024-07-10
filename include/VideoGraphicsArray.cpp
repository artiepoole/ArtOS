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

/*

Basic graphics utility methods

*/

#include "VideoGraphicsArray.h"

static VideoGraphicsArray* instance{nullptr};

/*

As the is no memory management the Offscreen buffer is allocated elsewhere and passed in.

This class could be re-worked as a Canvas with out to many changes */


VideoGraphicsArray::VideoGraphicsArray(const multiboot_header* boot_header, u32* buffer)
{
    [[maybe_unused]] auto& log = Serial::get();
    log.writeString("Initialising VGA. \n");
    instance = this;
    width = boot_header->framebuffer_width;
    height = boot_header->framebuffer_height;
    _screen = reinterpret_cast<u32*>(boot_header->framebuffer_addr);

    _buffer = buffer;

    // initialiase to 0
    for (u32 i = 0; i < (width * (height)); i++)
    {
        _buffer[i] = static_cast<u32>(0);
    }

    _window.x1 = 30;
    _window.y1 = 30;
    _window.x2 = width - (_window.x1);
    _window.y2 = height - (_window.y1);
    _window.w = _window.x2 - _window.x1;
    _window.h = _window.y2 - _window.y1;
    _screen_region = window_t{0, 0, width, height, width, height};

    log.writeString("VGA init done.\n");
}

VideoGraphicsArray::~VideoGraphicsArray()
{
    auto & log = Serial::get();
    log.writeString("VGA - Deconstructor called.");
    instance = nullptr;
}

VideoGraphicsArray& VideoGraphicsArray::get()
{
    return *instance;
}

void VideoGraphicsArray::putPixel(const u32 x, const u32 y, const u32 color) const
{
    if (x >= width || y >= height)
    {
        return;
    }
    _buffer[width * y + x] = color;
}

void VideoGraphicsArray::fillRectangle(const u32 x, const u32 y, const u32 w, const u32 h, const u32 color) const
{
    u32 i = width * (y - 1);

    // test if the Rectangle will be clipped (will it be fully in the screen or partially)
    if (x + w < width && y + h < height)
    {
        // fully drawn
        i += x + w;
        for (u32 yy = h; yy > 0; yy--)
        {
            i += width - w;
            for (u32 xx = w; xx > 0; xx--)
            {
                _buffer[i++] = color;
            }
        }
    }
    else
    {
        // clipped
        for (u32 yy = y; yy < y + h; yy++)
        {
            i += width;
            for (u32 xx = x; xx < x + w; xx++)
            {
                if (xx < width && yy < height)
                    _buffer[i + xx] = color;
            }
        }
    }
}

/**
 * Copy the screen buffer to the screen
 */
void VideoGraphicsArray::draw() const
{

    for (size_t i = 0; i < width * height; i++)
    {
        _screen[i] = _buffer[i];

    }


}

void VideoGraphicsArray::clearBuffer() const
{
    for (size_t i = 0; i < width * height; i++)
    {
        _buffer[i] = 0;
    }
}


void VideoGraphicsArray::drawSplash() const
{
    for (size_t ij = 0; ij < width * height; ij++)
    {
        _buffer[ij] = SPLASH_DATA[ij];
    }
}




window_t * VideoGraphicsArray::getScreen()
{
    return &_screen_region;
}


void VideoGraphicsArray::draw_region(const u32* buffer_to_draw) const
{
    for(size_t i=0; i<width*height; i++)
    {
        _screen[i] = buffer_to_draw[i];
    }
}

//
// // for each line from the second line onwards, copy next line into this line
// for (size_t x = 0; x < VGA_WIDTH; x++)
// {
//     // All lines move up one
//     for (size_t y = 0; y < VGA_HEIGHT - 1; y++)
//     {
//         size_t start_index = (y + 1) * VGA_WIDTH + x;
//         size_t end_index = (y) * (VGA_WIDTH) + x;
//         terminal_buffer[end_index] = terminal_buffer[start_index];
//     }
//     // Bottom line is replaced with empty.
//     terminal_buffer[VGA_WIDTH * (VGA_HEIGHT - 1) + x] = vga_entry(' ', terminal_color);
// }
// terminal_row -= 1;
// }
