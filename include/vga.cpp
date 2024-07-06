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


/*

As the is no memory management the Offscreen buffer is allocated elsewhere and passed in.

This class could be re-worked as a Canvas with out to many changes

*/
int char_dim = 8;
int font_scale = 1;
int scaled_char_dim = 8;
int window[4]; // x1, y1, x2, y2
int window_width;
int window_height;
u32 bkgd;
u32 frgd;

size_t terminal_row;
size_t terminal_column;
uint8_t terminal_color;
size_t buffer_dim[2];
char terminal_buffer[1024 / 8][768 / 8]; // 12288 characters total

VideoGraphicsArray::VideoGraphicsArray(const multiboot_header* boot_header, u32* _buffer)
{
    width = boot_header->framebuffer_width;
    height = boot_header->framebuffer_height;
    screen = reinterpret_cast<u32*>(boot_header->framebuffer_addr);

    buffer = _buffer;

    // initialiase to 0
    for (u32 i = 0; i < (width * (height)); i++)
    {
        buffer[i] = static_cast<u32>(0);
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


void VideoGraphicsArray::PutPixel(i32 x, i32 y, u32 color) const
{
    if (x < 0 || width <= x || y < 0 || height <= y)
        return;

    buffer[width * y + x] = color;
}

void VideoGraphicsArray::FillRectangle(i32 x, i32 y, u32 w, u32 h, u32 color) const
{
    u32 i = width * (y - 1);

    // test if the Rectangle will be clipped (will it be fully in the screen or partially)
    if (x >= 0 && x + w < width && y >= 0 && y + h < height)
    {
        // fully drawn
        i += x + w;
        for (i32 yy = h; yy > 0; yy--)
        {
            i += width - w;
            for (i32 xx = w; xx > 0; xx--)
            {
                buffer[i++] = color;
            }
        }
    }
    else
    {
        // clipped
        for (i32 yy = y; yy < y + h; yy++)
        {
            i += width;
            for (i32 xx = x; xx < x + w; xx++)
            {
                if (xx >= 0 && xx < width && yy >= 0 && yy < height)
                    buffer[i + xx] = color;
            }
        }
    }
}

/**
 * Copy the screen buffer to the screen
 */
void VideoGraphicsArray::bufferToScreen(bool clear) const
{
    // do the multiply once and test against 0
    for (int i = 0; i < width * height; i++)
    {
        screen[i] = buffer[i];
    }

    if (clear) clearBuffer();
}

void VideoGraphicsArray::clearBuffer() const
{
    for (int i = 0; i < width * height; i++)
    {
        buffer[i] = 0;
    }
}


void VideoGraphicsArray::PutChar(char ch, i32 x, i32 y, u32 color) const
{
    u32 px = 0; // position in the charactor - an 8x8 font = 64 bits
    u64 bCh = FONT[ch];

    // check if it will be drawn off screen
    if (x + (scaled_char_dim) < 0 || x > width || y + (scaled_char_dim) < 0 || y > height)
    {
        serial_write_string("Printing off screen\n");
        return;
    }

    // test if the charactor will be clipped (will it be fully in the screen or partially)
    if (x >= 0 && x + (scaled_char_dim) < width && y >= 0 && y + (scaled_char_dim) < height)
    {
        // fully in the screen - pre calculate some of the values
        // so there is less going on in the loop
        int i = width * (y - 1) + x + (scaled_char_dim); // start position for character
        int incAmount = width - (scaled_char_dim); // amount of pixels to get to start of next line of char
        for (int yy = 0; yy < (scaled_char_dim); yy++)
        {
            i += incAmount; // next character pixel line
            px = char_dim * (yy / font_scale); // select the correct pixel offset when repeating lines.

            for (int xx = 0; xx < (char_dim); xx++)
            {
                for (int s = 0; s < font_scale; s++)
                {
                    // test if a pixel is drawn here
                    if ((bCh >> (px)) & 1) // if the scale is >1, this should not change of shift each time
                    {
                        buffer[i] = color;
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
        int xpos = 0;
        int i = width * (y - 1);
        for (int yy = 0; yy < scaled_char_dim; yy++)
        {
            i += width;
            xpos = x;
            px = char_dim * (yy / font_scale);
            for (int xx = 0; xx < scaled_char_dim; xx++)
            {
                for (int s = 0; s < font_scale; s++)
                {
                    // test if a pixel is drawn here
                    if ((bCh >> (px)) & 1) // if the scale is >1, this should not change of shift each time
                    {
                        // Test if pixel pos on screen
                        if (xpos > 0 && xpos < width && yy + y > 0 && yy + y < height)
                            buffer[i + xpos] = color;
                    }
                    xpos++; // step along screen pixels
                }
                px++; // step along the character's pixel line
            }
        }
    }
}


void VideoGraphicsArray::PutStr(const char* ch, i32 x, i32 y, u32 color) const
{
    for (i32 i = 0; ch[i] != 0; i++, x += scaled_char_dim)
    {
        PutChar(ch[i], x, y, color);
    }
}

void VideoGraphicsArray::drawSplash() const
{
    for (size_t ij = 0; ij < width * height; ij++)
    {
        buffer[ij] = SPLASH_DATA[ij];
    }
}

void VideoGraphicsArray::setScale(const uint8_t new_scale) const
{
    // Checks if a character can be drawn in the region. Should be "window width" or something.
    if (new_scale * char_dim < width && new_scale * char_dim < height)
    {
        serial_write_string("New font scale: ");
        serial_write_int(new_scale);
        serial_new_line();
        font_scale = new_scale;
        scaled_char_dim = font_scale * char_dim;
        terminal_row = 0;
        terminal_column = 0;
        buffer_dim[0] = window_width / (scaled_char_dim);
        buffer_dim[1] = window_height / (scaled_char_dim);
    }
    else
    {
        serial_write_string("Font scale not applied\n");
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
        buffer[ij] = FRAME_DATA[ij];
    }
}

void VideoGraphicsArray::RenderTerminal() const
{
    auto x = window[0];
    auto y = window[1];

    for (size_t row = 0; row < buffer_dim[1] - 1; row++)
    {
        x = window[0];
        for (size_t col = 0; col < buffer_dim[0]; col++)
        {
            PutChar(terminal_buffer[col][row], col * scaled_char_dim + x, row * scaled_char_dim + y, frgd);
        }
    }
    bufferToScreen(false);
}

void VideoGraphicsArray::writeString(const char* data) const
{
    const int len = strlen(data);
    for (int i = 0; i < len; i++)
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
            scrollTerminal();
        }
    }
    RenderTerminal();
}



void VideoGraphicsArray::scrollTerminal() const
{
    clearWindow();
    for (size_t x = 0; x < buffer_dim[0]; x++)
    {
        // All lines move up one
        for (size_t y = 0; y < buffer_dim[1] - 1; y++)
        {
            terminal_buffer[x][y] = terminal_buffer[x][y+1];
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