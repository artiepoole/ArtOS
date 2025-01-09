#include "Terminal.h"

#include "cmp_int.h"
#include "ArtFile.h"
#include "Files.h"
#include "RTC.h"
#include "stdlib.h"
#include "string.h"
#include "time.h"


#include "VideoGraphicsArray.h"
#include "FONT.h"
#include "Serial.h"


#if FORLAPTOP
#define DEFAULT_SCALE 1
#else
#define DEFAULT_SCALE 2
#endif


static Terminal* term_instance{nullptr};
static TermFileWrapper* stdout_wrapper{nullptr};
static TermFileWrapper* stderr_wrapper{nullptr};
static bool owns_screen = false;

window_t screen_region = {0, 0, 1024, 768, 1024, 768};
u32 char_dim = 8;
u32 font_scale = DEFAULT_SCALE;
u32 scaled_char_dim = DEFAULT_SCALE * 8;
size_t terminal_row = 0;
size_t terminal_column = 1;
size_t cursor_idx = 1;
u32 buffer_width;
u32 buffer_height;
terminal_char_t* terminal_buffer;
terminal_char_t* rendered_buffer;

u32* term_screen_buffer;

constexpr size_t queue_size = 16384;
size_t queue_pos = 0;
terminal_char_t terminal_queue[queue_size];


Terminal::Terminal(const u32 x, const u32 y, const u32 width, const u32 height)
{
    screen_region = {x, y, width + x, height + y, width, height};
    buffer_width = screen_region.w / scaled_char_dim;
    buffer_height = screen_region.h / scaled_char_dim;
    const size_t n_characters = buffer_width * buffer_height;
    term_screen_buffer = static_cast<u32*>(malloc(screen_region.h * screen_region.w * sizeof(u32)));
    terminal_buffer = static_cast<terminal_char_t*>(malloc(n_characters * sizeof(terminal_char_t)));
    rendered_buffer = static_cast<terminal_char_t*>(malloc(n_characters * sizeof(terminal_char_t)));
    memset(terminal_buffer, 0, n_characters * sizeof(terminal_char_t));
    memset(rendered_buffer, 1, n_characters * sizeof(terminal_char_t));

    term_instance = this;

    for (size_t i = 0; i < screen_region.w * screen_region.h; i++)
    {
        term_screen_buffer[i] = colour_bkgd;
    }

    _render_queue(terminal_queue, MIN(queue_pos, n_characters));

    stdout_wrapper = new TermFileWrapper{false};

    stderr_wrapper = new TermFileWrapper{true};
    owns_screen = true;
}

Terminal::~Terminal()
{
    term_instance = nullptr;
}

Terminal& Terminal::get()
{
    return *term_instance;
}

ArtFile* Terminal::get_stdout_file()
{
    return stdout_wrapper->get_file();
}

ArtFile* Terminal::get_stderr_file()
{
    return stderr_wrapper->get_file();
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
    if (origin_x + scaled_char_dim < screen_region.x2 && origin_y + scaled_char_dim < screen_region.y2)
    {
        // fully in the screen_region
        size_t i = origin_x + screen_region.w * origin_y; // linear index
        const size_t line_step_amount = screen_region.w - scaled_char_dim; // amount to step to start next lin of character
        for (size_t y = 0; y < scaled_char_dim; y++)
        {
            for (size_t x = 0; x < scaled_char_dim; x++)
            {
                px = 8 * (y / font_scale) + (x / font_scale);

                term_screen_buffer[i++] = bCh >> px & 1 ? ch.colour : colour_bkgd;
            }
            i += line_step_amount;
        }
    }
    else
    {
        // partially in the screen_region
        size_t i = origin_x + screen_region.w * origin_y; // linear index
        const size_t line_step_amount = screen_region.w - min(screen_region.x2 - origin_x, scaled_char_dim);

        for (size_t y = 0; y < min(screen_region.y2 - origin_y, scaled_char_dim); y++)
        {
            for (size_t x = 0; x < min(screen_region.x2 - origin_x, scaled_char_dim); x++)
            {
                px = 8 * (y / font_scale) + (x / font_scale);

                term_screen_buffer[i++] = bCh >> px & 1 ? ch.colour : colour_bkgd;
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
    if (new_scale == 0) new_scale = DEFAULT_SCALE;

    if (new_scale * char_dim < screen_region.w && new_scale * char_dim < screen_region.h)
    {
        font_scale = new_scale;
        scaled_char_dim = font_scale * char_dim;
        terminal_row = 0;
        terminal_column = 1;

        buffer_width = screen_region.w / scaled_char_dim;
        buffer_height = screen_region.h / scaled_char_dim;
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
        write("\tFont scale not applied.\n", colour_error);
    }
}

void Terminal::setRegion(const u32 x, const u32 y, const u32 width, const u32 height)
{
    free(terminal_buffer);
    free(rendered_buffer);
    free(term_screen_buffer);
    screen_region = {x, y, width + x, height + y, width, height};
    buffer_width = screen_region.w / scaled_char_dim;
    buffer_height = screen_region.h / scaled_char_dim;
    terminal_buffer = static_cast<terminal_char_t*>(malloc(buffer_height * buffer_width * sizeof(terminal_char_t)));
    rendered_buffer = static_cast<terminal_char_t*>(malloc(buffer_height * buffer_width * sizeof(terminal_char_t)));
    term_screen_buffer = static_cast<u32*>(malloc(screen_region.h * screen_region.w * sizeof(u32)));
    // TODO: these are all being mapped to the same physical memory.
    memset(terminal_buffer, 0, buffer_height * buffer_width * sizeof(terminal_char_t));
    memset(rendered_buffer, 1, buffer_height * buffer_width * sizeof(terminal_char_t)); // all different
    memset(term_screen_buffer, 2, screen_region.h * screen_region.w * sizeof(u32));
    terminal_row = 0;
    terminal_column = 1;
    VideoGraphicsArray::get().fillRectangle(screen_region.x1, screen_region.y1, screen_region.w, screen_region.h, colour_bkgd);
    owns_screen = true;
}

u32 Terminal::getScale()
{
    return font_scale;
}

void Terminal::refresh()
{
    _draw_changes();
}

void Terminal::_update_cursor()
{
    size_t new_idx = terminal_row * buffer_width + terminal_column;
    if (new_idx >= buffer_width * buffer_height)
    {
        new_idx = buffer_width * buffer_height - 1;
    }
    if (cursor_idx >= new_idx && cursor_idx < buffer_width * buffer_height)
    {
        // backspace
        terminal_buffer[cursor_idx] = terminal_char_t{' ', colour_frgd};
    }
    cursor_idx = new_idx;
    terminal_buffer[cursor_idx] = terminal_char_t{'_', colour_frgd};
}

void Terminal::_draw_changes()
{
    if (!term_instance || !owns_screen) return;
    _update_cursor();
    size_t i = 0;
    for (size_t row = 0; row < buffer_height; row++)
    {
        for (size_t col = 0; col < buffer_width; col++)
        {
            if (terminal_char_t c_to_draw = terminal_buffer[i]; *reinterpret_cast<u64*>(&rendered_buffer[i]) != *reinterpret_cast<u64*>(&c_to_draw))
            {
                _putChar(c_to_draw, col * scaled_char_dim, row * scaled_char_dim);
                rendered_buffer[i] = c_to_draw;
            }

            ++i;
        }
    }
    const auto& vga = VideoGraphicsArray::get();
    vga.copy_region(term_screen_buffer, screen_region.x1, screen_region.y1, screen_region.w, screen_region.h);
    vga.draw();
}

void Terminal::_append_to_queue(const char* data, const u32 count, const PALETTE_t colour)
{
    for (size_t i = 0; i < count; i++)
    {
        terminal_queue[queue_pos++ % queue_size] = terminal_char_t{data[i], colour};
    }
}

void Terminal::_write_to_screen(const char* data, const u32 count, const PALETTE_t colour)
{
    if (!term_instance)
    {
        _append_to_queue(data, count, colour);
        return;
    }
    for (size_t i = 0; i < count; i++)
    {
        switch (const char c = data[i])
        {
        case '\t':
            {
                _write_to_screen("    ", 4, colour);

                if (terminal_column >= buffer_width) newLine();
                else cursor_idx += 4;
                break;
            }
        case '\n':
            {
                newLine();
                break;
            }
        default:
            {
                terminal_buffer[terminal_row * buffer_width + terminal_column++] = terminal_char_t{c, colour};

                if (terminal_column >= buffer_width) newLine();
                else cursor_idx += 1;
            }
        }

        if (terminal_row > buffer_height)
        {
            _scroll();
        }
    }
    _draw_changes();
}

u32 Terminal::user_write(const char* data, u32 count)
{
    PALETTE_t colour = colour_frgd;
    _write_to_screen(data, count, colour);
    return count;
}

u32 Terminal::user_err(const char* data, u32 count)
{
    PALETTE_t colour = colour_error;
    _write_to_screen(data, count, colour);
    return count;
}

u32 Terminal::write(bool b)
{
    if (b == true)
    {
        _write_to_screen("True", 4, colour_value);
        return 4;
    }
    _write_to_screen("False", 5, colour_value);
    return 5;
}

u32 Terminal::write(const char* data, PALETTE_t colour)
{
    const size_t len = mystrlen(data);
    // put data into the text buffer
    _write_to_screen(data, len, colour);
    return len;
}

u32 Terminal::write(const char c, const PALETTE_t colour)
{
    const char data[1] = {c};
    if (!term_instance)
    {
        _append_to_queue(data, 1, colour);
        return 1;
    }
    _write_to_screen(data, 1, colour);
    return 1;
}


u32 Terminal::write(const char* data, const size_t len, PALETTE_t colour)
{
    if (!term_instance)
    {
        _append_to_queue(data, len, colour);
        return len;
    }
    _write_to_screen(data, len, colour);
    return len;
}

// flush queue.
void Terminal::_render_queue(const terminal_char_t* data, size_t len)
{
    terminal_buffer[0] = terminal_char_t{'>', colour_accent};
    for (size_t i = 0; i < len; i++)
    {
        auto item = data[i];
        switch (data[i].letter)
        {
        case '\t':
            {
                write("    ", 4, data[i].colour);
                cursor_idx += 4;
                break;
            }
        case '\n':
            {
                newLine();
                break;
            }
        default:
            {
                //TODO: check on this becuase it goes out of bounds?
                terminal_buffer[terminal_row * buffer_width + terminal_column++] = item;
                cursor_idx++;
                break;
            }
        }
        if (terminal_column >= buffer_width)
        {
            newLine();
        }
    }
    _draw_changes();
}


void Terminal::backspace()
{
    if (!term_instance) return;
    if (terminal_column > 1)
    {
        terminal_buffer[terminal_row * buffer_width + (terminal_column - 1)] = terminal_char_t{' ', colour_bkgd};
        --terminal_column;
    }
    _draw_changes();
}

void Terminal::clear()
{
    if (!term_instance) return;
    memset(terminal_buffer, 0, buffer_height * buffer_width * sizeof(terminal_char_t));

    // 1 so that it doesn't match terminal buffer clears and redraws whole screen
    memset(rendered_buffer, 1, buffer_height * buffer_width * sizeof(terminal_char_t));
    _draw_changes();
}

void Terminal::stop_drawing()
{
    owns_screen = false;
}

void Terminal::resume_drawing()
{
    owns_screen = true;
}

void Terminal::time_stamp()
{
    tm time{};
    RTC::get().getTime(&time);
    write(asctime(&time));
}

// used to add and remove cursor
void Terminal::setChar(size_t x, size_t y, char c, PALETTE_t colour)
{
    if (term_instance)
    {
        terminal_buffer[y * buffer_width + x] = terminal_char_t{c, colour};
    }
}


void Terminal::newLine()
{
    if (!term_instance)
    {
        terminal_queue[queue_pos++ % queue_size] = terminal_char_t{'\n', colour_bkgd};
        return;
    }
    terminal_buffer[cursor_idx] = terminal_char_t{' ', colour_bkgd}; // remove cursor
    terminal_column = 1;
    terminal_row++;
    if (terminal_row >= buffer_height)
    {
        _scroll();
    }

    setChar(0, terminal_row - 1, ' ', colour_bkgd);
    setChar(0, terminal_row, '>', colour_accent);
    _draw_changes();
}

void Terminal::_scroll()
{
    // TODO: use px_x and px_y or row/column for this stuff and throughout the file for legibility reasons..
    if (!term_instance) return;
    for (size_t x = 0; x < buffer_width; x++)
    {
        // All lines move up one
        for (size_t y = 0; y < buffer_height - 1; y++)
        {
            terminal_buffer[y * buffer_width + x] = terminal_buffer[(y + 1) * buffer_width + x];
        }
        // Bottom line is replaced with empty.
        terminal_buffer[(buffer_height - 1) * buffer_width + x] = terminal_char_t{' ', colour_bkgd};
    }
    terminal_row -= 1;
}


TermFileWrapper::TermFileWrapper(const bool stderr): is_stderr(stderr)
{
    if (is_stderr)
    {
        strcpy(name, "stderr");
    }
    else
    {
        strcpy(name, "stdout");
    }

    file = ArtFile{this, name};
    register_storage_device(this);
}

ArtFile* TermFileWrapper::get_file()
{
    return &file;
}

i64 TermFileWrapper::write(const char* data, size_t, size_t byte_count)
{
    if (is_stderr) return Terminal::user_err(data, byte_count);
    return Terminal::user_write(data, byte_count);
}

ArtFile* TermFileWrapper::find_file(const char* filename)
{
    if (is_stderr)
    {
        if (constexpr char this_name[7] = "stderr"; strcmp(filename, this_name) == 0) return &file;
    }
    if (constexpr char this_name[7] = "stdout"; strcmp(filename, this_name) == 0) return &file;
    return nullptr;
}

char* TermFileWrapper::get_name()
{
    return name;
}
