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
 * Copy the frame buffer to the screen
 */
void VideoGraphicsArray::draw() const
{
    drawRegion(_buffer);
}

/*
 * Set the frame buffer to all zeros
 */
void VideoGraphicsArray::clearBuffer() const
{
    memset(_buffer, 0, width * height * sizeof(u32));
}

/*
 * Copies a rectangular buffer of shape (w,h) to screen starting with top left position (x,y) when value is not zero
 */
void VideoGraphicsArray::copy_region(const u32* src, const size_t x, const size_t y, const size_t w, const size_t h) const
{
    for (size_t yy = y; yy < y + h; yy++)
    {
        for (size_t xx = x; xx < x + w; xx++)
        {
            if (const u32 logo_pixel = src[(yy - y) * w + (xx - x)]; logo_pixel != 0)
            {
                _buffer[yy * width + xx] = logo_pixel;
            }
        }
    }
}


/*
 *  Draw the splash screen.
 */
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
    const u32 topleft_y = (height - splash_logo_height) / 3;
    copy_region(SPLASH_DATA, topleft_x, topleft_y, splash_logo_width, splash_logo_height);
}


window_t* VideoGraphicsArray::getScreen()
{
    return &_screen_region;
}


void VideoGraphicsArray::drawRegion(const u32* buffer_to_draw) const
{
    memcpy(_screen, buffer_to_draw, width * height * sizeof(u32));
}

/*
 * (x,y) origin of the total extents
 * (w,h) size of the total extents
 * border_width is subtracted to create the drawn region
 * n_chunks number of chunks for incrementing
 */
progress_bar_t VideoGraphicsArray::createProgressBar(u32 x, u32 y, u32 w, u32 h, u32 border, u32 n_chunks)
{
    if (border > 0) fillRectangle(x, y, w, h, colour_accent);
    fillRectangle(x + border, y + border, w - 2 * border, h - 2 * border, COLOR_BASE03);
    return progress_bar_t(x, y, w, h, border, n_chunks, 0);
}


void VideoGraphicsArray::setProgressBarPercent(progress_bar_t& bar, float percent)
{
    // reset whole bar
    fillRectangle(bar.x + bar.border, bar.y + bar.border, bar.w - 2 * bar.border, bar.h - 2 * bar.border, COLOR_BASE03);
    // fill up to percent adding 2px border
    if (percent > 100) percent = 100.;
    bar.chunk = percent * bar.n_chunks;
    if (percent == 0) return;
    fillRectangle(
        bar.x + bar.border + 2,
        bar.y + bar.border + 2,
        (bar.w - 2 * bar.border) * (percent / 100) - 4,
        bar.h - 2 * bar.border - 4, colour_frgd);
    draw();
}

void VideoGraphicsArray::setProgressBarChunk(progress_bar_t& bar, u32 chunk)
{
    // reset whole bar
    fillRectangle(bar.x + bar.border, bar.y + bar.border, bar.w - 2 * bar.border, bar.h - 2 * bar.border, COLOR_BASE03);

    if (chunk > bar.n_chunks) chunk = bar.n_chunks;
    bar.chunk = chunk;
    if (chunk == 0) return;
    // fill up to percent adding 2px border
    fillRectangle(
        bar.x + bar.border + 2,
        bar.y + bar.border + 2,
        (bar.w - 2 * bar.border) * (1.f * bar.chunk / bar.n_chunks) - 4,
        bar.h - 2 * bar.border - 4, colour_frgd);
    draw();
}

void VideoGraphicsArray::incrementProgressBarChunk(progress_bar_t& bar)
{
    setProgressBarChunk(bar, bar.chunk + 1);
}
