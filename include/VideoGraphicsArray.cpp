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

#include <string.h>

#include "stdlib.h"

#include "splash_screen.h"
#include "logging.h"

static VideoGraphicsArray* instance{nullptr};

/*

As the is no memory management the Offscreen buffer is allocated elsewhere and passed in.

This class could be re-worked as a Canvas with out to many changes */

u32* buffer;

VideoGraphicsArray::VideoGraphicsArray(const multiboot2_tag_framebuffer_common* framebuffer_info)
{
    instance = this;
    width = framebuffer_info->framebuffer_width;
    height = framebuffer_info->framebuffer_height;
    _screen = reinterpret_cast<u32*>(framebuffer_info->framebuffer_addr);

    _buffer = static_cast<u32*>(malloc(width * height * sizeof(u32)));
    memset(_buffer, 0, width * height * sizeof(u32));


    _screen_region = window_t{0, 0, width, height, width, height};

    LOG("VGA initialised with width: ", width, " and height: ", height);
}

VideoGraphicsArray::~VideoGraphicsArray()
{
    // WRITE("VGA - Deconstructor called.");
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
    memcpy(_screen, buffer_to_draw, width * height * sizeof(u32));
}

