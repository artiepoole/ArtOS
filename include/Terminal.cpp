#include "Terminal.h"

#include <RTC.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <doomgeneric/i_video.h>

#include "VideoGraphicsArray.h"
#include "FONT.h"
#include "Serial.h"
#include "doomgeneric/d_think.h"

static Terminal* instance{nullptr};

window_t screen_region = {0, 0, 1024, 768, 1024, 768};
u32 char_dim = 8;
u32 font_scale = 2;
u32 scaled_char_dim = 16;
size_t terminal_row = 0;
size_t terminal_column = 0;
u32 buffer_width;
u32 buffer_height;
terminal_char_t* terminal_buffer; // 12288 characters total
terminal_char_t* rendered_buffer; // 12288 characters total

u32* term_screen_buffer;

constexpr size_t queue_size = 16384;
size_t queue_pos = 0;
terminal_char_t terminal_queue[queue_size];


Terminal::Terminal(u32 width, u32 height)
{
    screen_region = {0, 0, width, height, width, height};
    buffer_width = screen_region.w / (scaled_char_dim);
    buffer_height = screen_region.h / (scaled_char_dim);
    term_screen_buffer = static_cast<u32*>(malloc(screen_region.h * screen_region.w * sizeof(u32)));
    terminal_buffer = static_cast<terminal_char_t*>(malloc(buffer_height * buffer_width * sizeof(terminal_char_t)));
    rendered_buffer = static_cast<terminal_char_t*>(malloc(buffer_height * buffer_width * sizeof(terminal_char_t)));
    memset(terminal_buffer, 0, buffer_height * buffer_width * sizeof(terminal_char_t));
    memset(rendered_buffer, 0, buffer_height * buffer_width * sizeof(terminal_char_t));

    instance = this;

    for (size_t i = 0; i < screen_region.w * screen_region.h; i++)
    {
        term_screen_buffer[i] = bkgd;
    }

    write(terminal_queue, queue_pos);
    // delete terminal_queue;
}

Terminal::~Terminal()
{
    instance = nullptr;
}

Terminal& Terminal::get()
{
    return *instance;
}

size_t min(size_t a, size_t b)
{
    if (a < b) return a;
    return b;
}

void Terminal::_putChar(const terminal_char_t ch, const u32 origin_x, const u32 origin_y)
{
    u32 px; // position in the charactor - an 8x8 font = 64 bits
    const u64 bCh = FONT[static_cast<size_t>(ch.letter)];

    // test if the charactor will be clipped (will it be fully in the screen_region or partially)
    if (origin_x + (scaled_char_dim) < screen_region.x2 && origin_y + (scaled_char_dim) < screen_region.y2)
    {
        // fully in the screen_region
        size_t i = origin_x + screen_region.w * origin_y; // linear index
        const size_t line_step_amount = screen_region.w - scaled_char_dim; // amount to step to start next lin of character
        for (size_t y = 0; y < (scaled_char_dim); y++)
        {
            for (size_t x = 0; x < (scaled_char_dim); x++)
            {
                px = 8 * (y / font_scale) + (x / font_scale);

                if ((bCh >> (px)) & 1)
                {
                    term_screen_buffer[i++] = ch.colour;
                }
                else
                {
                    term_screen_buffer[i++] = bkgd;
                }
            }
            i += line_step_amount;
        }
    }
    else
    {
        // patially in the screen_region
        size_t i = origin_x + screen_region.w * origin_y; // linear index
        const size_t line_step_amount = screen_region.w - min(screen_region.x2 - origin_x, scaled_char_dim);

        for (size_t y = 0; y < min(screen_region.y2 - origin_y, scaled_char_dim); y++)
        {
            for (size_t x = 0; x < min(screen_region.x2 - origin_x, scaled_char_dim); x++)
            {
                px = 8 * (y / font_scale) + (x / font_scale);

                if ((bCh >> (px)) & 1)
                {
                    term_screen_buffer[i++] = ch.colour;
                }
                else
                {
                    term_screen_buffer[i++] = bkgd;
                }
            }
            i += line_step_amount;
        }
    }
}


void Terminal::setScale(u32 new_scale)
{
    // 0 is default: 2
    // LOG("Terminal: Setting font scale to ", new_scale, " from ", font_scale);
    // Checks if a character can be drawn in the region. Should be "_window width" or something.
    if (new_scale == 0) new_scale = 2;

    if (new_scale * char_dim < screen_region.w && new_scale * char_dim < screen_region.h)
    {
        font_scale = new_scale;
        scaled_char_dim = font_scale * char_dim;
        terminal_row = 0;
        terminal_column = 1;

        buffer_width = screen_region.w / (scaled_char_dim);
        buffer_height = screen_region.h / (scaled_char_dim);
        free(terminal_buffer);
        free(rendered_buffer);

        terminal_buffer = static_cast<terminal_char_t*>(malloc(buffer_height * buffer_width * sizeof(terminal_char_t)));
        rendered_buffer = static_cast<terminal_char_t*>(malloc(buffer_height * buffer_width * sizeof(terminal_char_t)));
        memset(terminal_buffer, 0, buffer_height * buffer_width * sizeof(terminal_char_t));

        // 1 so that it doesn't match terminal buffer clears and redraws whole screen
        memset(rendered_buffer, 1, buffer_height * buffer_width * sizeof(terminal_char_t));
        time_stamp();
        write("Font scale set to: ");
        write(font_scale, false);
        newLine();
    }
    else
    {
        time_stamp();
        write("\tFont scale not applied.\n", error);
    }
}

u32 Terminal::getScale()
{
    return font_scale;
}

void Terminal::_render()
{
    if (!instance) return;
    size_t i = 0;
    for (size_t row = 0; row < buffer_height; row++)
    {
        for (size_t col = 0; col < buffer_width; col++)
        {
            const terminal_char_t c_to_draw = terminal_buffer[i];
            const terminal_char_t c_drawn = rendered_buffer[i];

            if (c_to_draw.letter != c_drawn.letter)
            {
                _putChar(c_to_draw, col * scaled_char_dim, row * scaled_char_dim);
            }
            ++i;
        }
    }
    const auto& vga = VideoGraphicsArray::get();
    vga.draw_region(term_screen_buffer);
}


// void Terminal::write(const char* data, const u32 colour)
// {
//     const size_t len = mystrlen(data);
//     // put data into the text buffer
//     write(data, len, colour);
// }

void Terminal::write(const char* data, PALETTE_t colour)
{
    const size_t len = mystrlen(data);
    // put data into the text buffer
    write(data, len, colour);
}


// void Terminal::write(const char* data)
// {
//     const size_t len = mystrlen(data);
//     // put data into the text buffer
//     write(data, len, COLOR_BASE0);
// }

void Terminal::write(const char c, const PALETTE_t colour)
{
    if (!instance)
    {
        terminal_queue[queue_pos++ % queue_size] = terminal_char_t{c, colour};
        return;
    }
    if (c == '\n')
    {
        newLine();
        terminal_buffer[terminal_row * buffer_width] = terminal_char_t{'>', accent};
    }
    else
    {
        terminal_buffer[terminal_row * buffer_width + terminal_column++] = terminal_char_t{c, colour};
    }
    if (terminal_column >= buffer_width)
    {
        newLine();
    }
    _render();
}


void Terminal::write(const char* data, const size_t len, PALETTE_t color)
{
    if (!instance)
    {
        for (int i = 0; i < len; i++)
        {
            terminal_queue[queue_pos++ % queue_size] = terminal_char_t{data[i], color};
        }
        return;
    }

    for (size_t i = 0; i < len; i++)
    {
        const char c = data[i];
        switch (c)
        {
        case '\t':
            {
                write("    ", 4, color);
                if (terminal_column >= buffer_width) newLine();
                break;
            }
        case '\n':
            {
                newLine();
                break;
            }
        default:
            {
                terminal_buffer[terminal_row * buffer_width + terminal_column++] = terminal_char_t{c, color};;

                if (terminal_column >= buffer_width) newLine();
            }
        }

        if (terminal_row > buffer_height)_scroll();
        // write the text buffer to screen_region
    }
    _render();
}

// flush queue.
void Terminal::write(const terminal_char_t* data, size_t len)
{
    terminal_buffer[0] = terminal_char_t{'>', accent};
    for (size_t i = 0; i < len; i++)
    {
        auto item = data[i];
        switch (data[i].letter)
        {
        case '\t':
            {
                write("    ", 4, data[i].colour);
                break;
            }
        case '\n':
            {
                newLine();
                break;
            }
        default:
            {
                terminal_buffer[terminal_row * buffer_width + terminal_column++] = item;
                break;
            }
        }
        if (terminal_column >= buffer_width)
        {
            newLine();
        }
    }
    _render();
}


void Terminal::backspace()
{
    if (terminal_column > 1)
    {
        terminal_buffer[terminal_row * buffer_width + (terminal_column - 1)] = terminal_char_t{' ', bkgd};;
        --terminal_column;
    }
    _render();
}

void Terminal::clear()
{
    memset(terminal_buffer, 0, buffer_height * buffer_width * sizeof(terminal_char_t));

    // 1 so that it doesn't match terminal buffer clears and redraws whole screen
    memset(rendered_buffer, 1, buffer_height * buffer_width * sizeof(terminal_char_t));
    _render();
}

void Terminal::time_stamp()
{
    write(asctime(RTC::get().getTime()));
}

// used to add and remove cursor
void Terminal::_setChar(size_t x, size_t y, char c, PALETTE_t colour)
{
    terminal_buffer[y * buffer_width + x] = terminal_char_t{c, colour};
}

void Terminal::userLine() // for use after an application prints.
{
    newLine();
    terminal_buffer[terminal_row * buffer_width] = terminal_char_t{'>', accent}; // Add an arrow to the start of the line
    _render();
}

void Terminal::newLine()
{
    if (!instance)
    {
        terminal_queue[queue_pos++ % queue_size] = terminal_char_t{'\n', bkgd};
    }
    terminal_column = 1;
    terminal_row++;
    if (terminal_row >= buffer_height)
    {
        _scroll();
    }

    _setChar(0, terminal_row - 1, ' ', bkgd);
    _setChar(0, terminal_row, '>', accent);
}

void Terminal::_scroll()
{
    // TODO: use px_x and px_y or row/column for this stuff and throughout the file for legibility reasons..
    // auto& vga = VideoGraphicsArray::get();
    // vga.clearWindow();
    for (size_t x = 0; x < buffer_width; x++)
    {
        // All lines move up one
        for (size_t y = 0; y < buffer_height - 1; y++)
        {
            terminal_buffer[y * buffer_width + x] = terminal_buffer[(y + 1) * buffer_width + x];
        }
        // Bottom line is replaced with empty.
        terminal_buffer[(buffer_height - 1) * buffer_width + x] = terminal_char_t{' ', bkgd};
    }
    terminal_row -= 1;
}
