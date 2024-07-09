#include "Terminal.h"


static Terminal* instance{nullptr};

u32 char_dim = 8;
u32 font_scale = 1;
u32 scaled_char_dim = 8;
size_t terminal_row = 0;
size_t terminal_column = 0;
u32 bkgd;
u32 frgd;
u32 buffer_width;
u32 buffer_height;
char terminal_buffer[1024 / 8][768 / 8] = {0}; // 12288 characters total
char rendered_buffer[1024 / 8][768 / 8] = {0}; // 12288 characters total

Terminal::Terminal(window_t screen, window_t window)
{
    instance = this;
    _screen = screen;
    _window = window;
    bkgd = COLOR_BASE02;
    frgd = COLOR_BASE0;
    buffer_width = _window.w / (scaled_char_dim);
    buffer_height = _window.h / (scaled_char_dim);
}

Terminal::~Terminal()
{
    instance = nullptr;
}

Terminal& Terminal::get()
{
    return *instance;
}


void Terminal::_putChar(const char ch, const u32 origin_x, const u32 origin_y) const
{
    // todo: i is useless as it is a linear index and I need to calculate pixel locations instead.


    auto& vgap = VideoGraphicsArray::get();
    [[maybe_unused]] auto& log = Serial::get();
    u32 px; // position in the charactor - an 8x8 font = 64 bits
    const u64 bCh = FONT[static_cast<size_t>(ch)];


    // test if the charactor will be clipped (will it be fully in the screen or partially)
    if (origin_x + (scaled_char_dim) < _window.x2 && origin_y + (scaled_char_dim) < _window.y2)
    {
        // fully in the screen
        for (size_t y = 0; y < (scaled_char_dim); y++)
        {
            for (size_t x = 0; x < (scaled_char_dim); x++)
            {
                px = 8 * (y / font_scale) + (x / font_scale);

                if ((bCh >> (px)) & 1)
                {
                    vgap.putPixel(origin_x + x, origin_y + y, frgd);
                }
                else
                {
                    vgap.putPixel(origin_x + x, origin_y + y, bkgd);
                }
            }
        }
        // vgap.draw();
    }
    else
    {
        // patially in the screen
        for (size_t y = 0; y < _window.y2; y++)
        {
            for (size_t x = 0; x < _window.x2; x++)
            {
                px = 8 * (y / font_scale) + (x / font_scale);

                if ((bCh >> (px)) & 1)
                {
                    vgap.putPixel(origin_x + x, origin_y + y, frgd);
                }
                else
                {
                    vgap.putPixel(origin_x + x, origin_y + y, bkgd);
                }
            }
        }

    }

}


void Terminal::setScale(const u32 new_scale)
{
    auto& log = Serial::get();
    // Checks if a character can be drawn in the region. Should be "_window width" or something.
    if (new_scale * char_dim < _screen.w && new_scale * char_dim < _screen.h)
    {
        log.writeString("New font scale: ");
        log.writeInt(new_scale);
        log.newLine();
        font_scale = new_scale;
        scaled_char_dim = font_scale * char_dim;
        terminal_row = 0;
        terminal_column = 0;
        buffer_width = _window.w / (scaled_char_dim);
        buffer_height = _window.h / (scaled_char_dim);
    }
    else
    {
        log.writeString("Font scale not applied\n");
    }
}

u32 Terminal::getScale()
{
    return font_scale;
}

void Terminal::_render() const
{

    const auto y = _window.y1;
    const auto x = _window.x1;
    for (size_t row = 0; row < buffer_height; row++)
    {
        for (size_t col = 0; col < buffer_width; col++)
        {
            const char c_to_draw = terminal_buffer[col][row];
            const char c_drawn = rendered_buffer[col][row];

            if (c_to_draw!= c_drawn)
            {
                _putChar(c_to_draw, col * scaled_char_dim + x, row * scaled_char_dim + y);
            }
        }
    }
    const auto& vga = VideoGraphicsArray::get();
    vga.draw();
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

void Terminal::writeString(const char* data)
{

    const size_t len = strlen(data);
    // put data into the text buffer
    writeBuffer(data, len);

}

void Terminal::writeChar(const char c)
{
    if (c == '\n')
    {
        newLine();
        terminal_buffer[0][terminal_row] = '>';
    }
    else
    {
        terminal_buffer[terminal_column++][terminal_row] = c;
    }
    if (terminal_column >= buffer_width)
    {
        newLine();
    }
    _render();
}


void Terminal::_scroll()
{
    auto& vga = VideoGraphicsArray::get();
    vga.clearWindow();
    for (size_t x = 0; x < buffer_width; x++)
    {
        // All lines move up one
        for (size_t y = 0; y < buffer_height - 1; y++)
        {
            terminal_buffer[x][y] = terminal_buffer[x][y + 1];
        }
        // Bottom line is replaced with empty.
        terminal_buffer[x][buffer_height - 1] = ' ';
    }
    terminal_row -= 1;
}


void Terminal::writeBuffer(const char* data, size_t len)
{
    for (size_t i = 0; i < len; i++)
    {
        if (const char c = data[i]; c == '\n')
        {
            newLine();
            terminal_buffer[0][terminal_row] = '>';
        }
        else
        {
            terminal_buffer[terminal_column++][terminal_row] = c;

            if (terminal_column >= buffer_width) newLine();
        }
        if (terminal_row > buffer_height)_scroll();
        // write the text buffer to screen
        _render();
    }
}

void Terminal::backspace() const
{
    if (terminal_column > 1)
    {
        terminal_buffer[terminal_column - 1][terminal_row] = ' ';
        --terminal_column;
    }
    _render();
}
