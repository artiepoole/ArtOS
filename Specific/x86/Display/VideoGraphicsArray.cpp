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

#include "SIMD.h"
#include "CPU.h"
#include <string.h>

#include "stdlib.h"

#include "splash_screen.h"
#include "logging.h"
#include "colours.h"


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
    draw_region(_buffer);
}

void VideoGraphicsArray::clearBuffer() const
{
    memset(_buffer, 0, width * height * sizeof(u32));
}

void copy_region(u32* dest, const u32* src, const size_t start_x, const size_t start_y, const size_t dest_w, const size_t src_w, const size_t src_h)
{
    for (size_t line = start_y; line < start_y + src_h; line++)
    {
        for (size_t px = start_x; px < start_x + src_w; px++)
        {
            if (const u32 logo_pixel = src[(line - start_y) * src_w + (px - start_x)]; logo_pixel != 0)
            {
                dest[line * dest_w + px] = logo_pixel;
            }
        }
    }
}

void VideoGraphicsArray::drawSplash() const
{
    // Step 1: fill the screen with background colour
    fillRectangle(0, 0, width, height, COLOR_BASE02);

    // Step 2: Draw borders
    constexpr u32 dark_border_width = 35;
    constexpr u32 light_border_width = 10;
    // left and top dark
    fillRectangle(0, 0, width, dark_border_width, COLOR_BASE03);
    fillRectangle(0, 0, dark_border_width, height, COLOR_BASE03);
    // right and bottom dark
    fillRectangle(0, height - dark_border_width, width, dark_border_width, COLOR_BASE03);
    fillRectangle(width - dark_border_width, 0, dark_border_width, height, COLOR_BASE03);
    // left and top light
    fillRectangle(dark_border_width, dark_border_width, width - dark_border_width * 2, light_border_width, COLOR_CYAN);
    fillRectangle(dark_border_width, dark_border_width, light_border_width, height - dark_border_width * 2, COLOR_CYAN);
    // right and bottom light
    fillRectangle(dark_border_width, height - dark_border_width - light_border_width, width - dark_border_width * 2, light_border_width, COLOR_CYAN);
    fillRectangle(width - dark_border_width - light_border_width, dark_border_width, light_border_width, height - dark_border_width * 2, COLOR_CYAN);

    // Step 3: copy the splash logo
    const u32 topleft_x = (width - splash_logo_width) / 2;
    const u32 topleft_y = (height - splash_logo_height) / 2;
    copy_region(_buffer, SPLASH_DATA, topleft_x, topleft_y, width, splash_logo_width, splash_logo_height);
}


window_t
*
VideoGraphicsArray::getScreen()
{
    return &_screen_region;
}


void VideoGraphicsArray::draw_region(const u32* buffer_to_draw) const
{
    memcpy(_screen, buffer_to_draw, width * height * sizeof(u32));
}
