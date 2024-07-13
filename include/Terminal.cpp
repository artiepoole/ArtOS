#include "Terminal.h"


static Terminal* instance{nullptr};

window_t screen_region = {0, 0, 1024, 768, 1024, 768};
u32 char_dim = 8;
u32 font_scale = 1;
u32 scaled_char_dim = 8;
size_t terminal_row = 0;
size_t terminal_column = 0;
u32 bkgd;
u32 frgd;
u32 accent;
u32 error;
u32 buffer_width;
u32 buffer_height;
terminal_char_t terminal_buffer[1024 / 8 * 768 / 8]; // 12288 characters total
terminal_char_t rendered_buffer[1024 / 8 * 768 / 8]; // 12288 characters total

u32 term_screen_buffer[1024 * 768];

Terminal::Terminal()
{
    instance = this;
    bkgd = COLOR_BASE02;
    frgd = COLOR_BASE0;
    accent = COLOR_CYAN;
    error = COLOR_RED;
    for (size_t i = 0; i < screen_region.w * screen_region.h; i++)
    {
        term_screen_buffer[i] = bkgd;
    }
    // buffer_width = screen_region.w / (scaled_char_dim);
    // buffer_height = screen_region.h / (scaled_char_dim);

    // term_screen_buffer = reinterpret_cast<u32*>(malloc(screen_region.w * screen_region.h * 4));
    // terminal_buffer = reinterpret_cast<terminal_char_t*>(malloc(screen_region.w / scaled_char_dim * screen_region.h / scaled_char_dim * sizeof(terminal_char_t)));
    // rendered_buffer = reinterpret_cast<terminal_char_t*>(malloc(screen_region.w / scaled_char_dim * screen_region.h / scaled_char_dim * sizeof(terminal_char_t)));
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
                    term_screen_buffer[i++] = ch.color;
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
                    term_screen_buffer[i++] = ch.color;
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


void Terminal::setScale(const u32 new_scale)
{
    auto& log = Serial::get();
    // Checks if a character can be drawn in the region. Should be "_window width" or something.
    if (new_scale * char_dim < screen_region.w && new_scale * char_dim < screen_region.h)
    {
        log.write("New font scale: ");
        log.write(new_scale);
        log.newLine();
        font_scale = new_scale;
        scaled_char_dim = font_scale * char_dim;
        terminal_row = 0;
        terminal_column = 0;
        buffer_width = screen_region.w / (scaled_char_dim);
        buffer_height = screen_region.h / (scaled_char_dim);
    }
    else
    {
        log.write("Font scale not applied\n");
    }
}

u32 Terminal::getScale()
{
    return font_scale;
}

void Terminal::_render()
{
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

void Terminal::newLine()
{
    terminal_column = 1;
    terminal_row++;
    if (terminal_row >= buffer_height)
    {
        _scroll();
    }
}

void Terminal::userLine() // for use after an application prints.
{
    newLine();
    terminal_buffer[terminal_row * buffer_width] = terminal_char_t{'>', accent}; // Add an arrow to the start of the line
    _render();
}



void Terminal::write(const char* data, const u32 color)
{
    const size_t len = mystrlen(data);
    // put data into the text buffer
    write(data, len, color);
}

void Terminal::write(const char c, const u32 color)
{
    if (c == '\n')
    {
        newLine();
        terminal_buffer[terminal_row * buffer_width] = terminal_char_t{'>', accent};;
    }
    else
    {
        terminal_buffer[terminal_row * buffer_width + terminal_column++] = terminal_char_t{c, color};
    }
    if (terminal_column >= buffer_width)
    {
        newLine();
    }
    _render();
}


void Terminal::_scroll()
{
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


void Terminal::write(const char* data, const size_t len, const u32 color)
{
    for (size_t i = 0; i < len; i++)
    {
        if (const char c = data[i]; c == '\n')
        {
            newLine();
            terminal_buffer[terminal_row * buffer_width] = terminal_char_t{'>', accent};
        }
        else
        {
            terminal_buffer[terminal_row * buffer_width + terminal_column++] = terminal_char_t{c, color};;

            if (terminal_column >= buffer_width) newLine();
        }
        if (terminal_row > buffer_height)_scroll();
        // write the text buffer to screen_region
        _render();
    }
}


void Terminal::backspace()
{
    if (terminal_column > 1)
    {
        terminal_buffer[terminal_row * buffer_width + (terminal_column - 1)] = terminal_char_t{'>', bkgd};;
        --terminal_column;
    }
    _render();
}

void Terminal::clear()
{
    // Todo: implement a clear terminal function
    return;
}
