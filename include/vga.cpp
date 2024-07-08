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

#include "vga.h"
static VideoGraphicsArray* instance{nullptr};

/*

As the is no memory management the Offscreen buffer is allocated elsewhere and passed in.

This class could be re-worked as a Canvas with out to many changes */


u32 char_dim = 8;
u32 font_scale = 1;
u32 scaled_char_dim = 8;
u32 window[4]; // x1, y1, x2, y2
u32 window_width;
u32 window_height;
u32 bkgd;
u32 frgd;

size_t terminal_row;
size_t terminal_column;
uint8_t terminal_color;
size_t buffer_dim[2];
char terminal_buffer[1024 / 8][768 / 8]; // 12288 characters total

VideoGraphicsArray::VideoGraphicsArray(const multiboot_header* boot_header, u32* buffer)
{
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

    window[0] = 30;
    window[1] = 30;
    window[2] = width - (window[0]);
    window[3] = height - (window[1]);
    window_width = window[2] - window[0];
    window_height = window[3] - window[1];

    bkgd = COLOR_BASE02;
    frgd = COLOR_BASE0;

    terminal_row = 0;
    terminal_column = 0;
    buffer_dim[0] = window_width / (scaled_char_dim);
    buffer_dim[1] = window_height / (scaled_char_dim);
}

VideoGraphicsArray::~VideoGraphicsArray()
{
    instance = nullptr;
}

VideoGraphicsArray& VideoGraphicsArray::get()
{
    return *instance;
}

void VideoGraphicsArray::putPixel(u32 x, u32 y, u32 color) const
{
    if (width <= x || height <= y)
        return;

    _buffer[width * y + x] = color;
}

void VideoGraphicsArray::fillRectangle(const u32 x, const u32 y, const u32 w, const u32 h, const u32 color) const
{
    u32 i = width * (y - 1);

    // test if the Rectangle will be clipped (will it be fully in the screen or partially)
    if ( x + w < width &&  y + h < height)
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
                if (xx < width  && yy < height)
                    _buffer[i + xx] = color;
            }
        }
    }
}

/**
 * Copy the screen buffer to the screen
 */
void VideoGraphicsArray::bufferToScreen(const bool clear) const
{
    // do the multiply once and test against 0
    for (size_t i = 0; i < width * height; i++)
    {
        _screen[i] = _buffer[i];
    }

    if (clear) clearBuffer();
}

void VideoGraphicsArray::clearBuffer() const
{
    for (size_t i = 0; i < width * height; i++)
    {
        _buffer[i] = 0;
    }
}


void VideoGraphicsArray::putChar(const char ch, const u32 x, const u32 y, const u32 color) const
{
    auto& log = Serial::get();
    u32 px; // position in the charactor - an 8x8 font = 64 bits
    const u64 bCh = FONT[static_cast<size_t>(ch)];

    // check if it will be drawn off screen
    if (x > width ||  y > height)
    {
        log.writeString("Printing off screen\n");
        return;
    }

    // test if the charactor will be clipped (will it be fully in the screen or partially)
    if ( x + (scaled_char_dim) < width &&  y + (scaled_char_dim) < height)
    {
        // fully in the screen - pre calculate some of the values
        // so there is less going on in the loop
        size_t i = width * (y - 1) + x + (scaled_char_dim); // start position for character
        size_t incAmount = width - (scaled_char_dim); // amount of pixels to get to start of next line of char
        for (size_t yy = 0; yy < (scaled_char_dim); yy++)
        {
            i += incAmount; // next character pixel line
            px = char_dim * (yy / font_scale); // select the correct pixel offset when repeating lines.

            for (size_t xx = 0; xx < (char_dim); xx++)
            {
                for (size_t s = 0; s < font_scale; s++)
                {
                    // test if a pixel is drawn here
                    if ((bCh >> (px)) & 1) // if the scale is >1, this should not change of shift each time
                    {
                        _buffer[i] = color; //  vga.putpixel(x,y);
                    }
                    i++; // step along screen pixels
                }
                px++; // step along the character's pixel line
            }
        }
    }
    else
    {
        // partially in the screen
        size_t i = width * (y - 1);
        for (size_t yy = 0; yy < scaled_char_dim; yy++)
        {
            i += width;
            size_t xpos = x;
            px = char_dim * (yy / font_scale);
            for (size_t xx = 0; xx < scaled_char_dim; xx++)
            {
                for (size_t s = 0; s < font_scale; s++)
                {
                    // test if a pixel is drawn here
                    if ((bCh >> (px)) & 1) // if the scale is >1, this should not change of shift each time
                    {
                        // Test if pixel pos on screen
                        if (xpos > 0 && xpos < width && yy + y > 0 && yy + y < height)
                            _buffer[i + xpos] = color;
                    }
                    xpos++; // step along screen pixels
                }
                px++; // step along the character's pixel line
            }
        }
    }
}


void VideoGraphicsArray::putStr(const char* ch,  u32 x, const u32 y, const u32 color) const
{
    for (u32 i = 0; ch[i] != 0; i++, x += scaled_char_dim)
    {
        putChar(ch[i], x, y, color);
    }
}

void VideoGraphicsArray::drawSplash() const
{
    for (size_t ij = 0; ij < width * height; ij++)
    {
        _buffer[ij] = SPLASH_DATA[ij];
    }
}

void VideoGraphicsArray::setScale(const uint8_t new_scale) const
{
    auto& log = Serial::get();
    // Checks if a character can be drawn in the region. Should be "window width" or something.
    if (new_scale * char_dim < width && new_scale * char_dim < height)
    {
        log.writeString("New font scale: ");
        log.writeInt(new_scale);
        log.newLine();
        font_scale = new_scale;
        scaled_char_dim = font_scale * char_dim;
        terminal_row = 0;
        terminal_column = 0;
        buffer_dim[0] = window_width / (scaled_char_dim);
        buffer_dim[1] = window_height / (scaled_char_dim);
    }
    else
    {
        log.writeString("Font scale not applied\n");
    }
}

u8 VideoGraphicsArray::getScale()
{
    return font_scale;
}

void VideoGraphicsArray::clearWindow() const
{
    for (size_t ij = 0; ij < width * height; ij++)
    {
        _buffer[ij] = FRAME_DATA[ij];
    }
}

void VideoGraphicsArray::_renderTerminal() const
{
    const auto y = window[1];

    for (size_t row = 0; row < buffer_dim[1] - 1; row++)
    {
        u32 x = window[0];
        for (size_t col = 0; col < buffer_dim[0]; col++)
        {
            putChar(terminal_buffer[col][row], col * scaled_char_dim + x, row * scaled_char_dim + y, frgd);
        }
    }
    bufferToScreen(false);
}

void VideoGraphicsArray::writeString(const char* data) const
{
    const size_t len = strlen(data);
    for (size_t i = 0; i < len; i++)
    {
        char c = data[i];
        if (c == '\n')
        {
            terminal_row++;
            terminal_column = 0;
        }
        else
        {
            terminal_buffer[terminal_column++][terminal_row] = c;
        }
        if (terminal_column >= buffer_dim[0])
        {
            terminal_column = 0;
            terminal_row++;
        }
        if (terminal_row >= buffer_dim[1])
        {
            _scrollTerminal();
        }
    }
    _renderTerminal();
}

void VideoGraphicsArray::writeChar(char c) const
{
    if (c == '\n')
    {
        terminal_row++;
        terminal_column = 0;
    }
    else
    {
        terminal_buffer[terminal_column++][terminal_row] = c;
    }
    if (terminal_column >= buffer_dim[0])
    {
        terminal_column = 0;
        terminal_row++;
    }
    if (terminal_row >= buffer_dim[1])
    {
        _scrollTerminal();
    }

    _renderTerminal();
}


void VideoGraphicsArray::_scrollTerminal() const
{
    clearWindow();
    for (size_t x = 0; x < buffer_dim[0]; x++)
    {
        // All lines move up one
        for (size_t y = 0; y < buffer_dim[1] - 1; y++)
        {
            terminal_buffer[x][y] = terminal_buffer[x][y + 1];
        }
        // Bottom line is replaced with empty.
        terminal_buffer[x][buffer_dim[1]] = ' ';
    }
    terminal_row -= 1;
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
