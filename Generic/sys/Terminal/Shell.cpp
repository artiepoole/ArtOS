//
// Created by artypoole on 22/11/24.
//

#include "Shell.h"

#include <kernel.h>
#include <string.h>

extern "C" {
#include "doomgeneric/doomgeneric.h"
}

#include "Terminal.h"
#include "EventQueue.h"

Shell::Shell(EventQueue* e)
{
    events = e;
}


void Shell::run()
{
    while (true)
    {
        if (events->pendingEvents())
        {
            auto [type, data] = events->getEvent();
            // LOG("Found event. Type: ", static_cast<int>(type), " lower: ", data.lower_data, " upper: ",data.upper_data);
            switch (type)
            {
            case NULL_EVENT:
                {
                    Terminal::write("NULL EVENT\n");
                    break;
                }
            case KEY_UP:
                {
                    const size_t cin = data.lower_data;
                    const char key = key_map[cin];

                    if (key_map[cin] != 0)
                    {
                        switch (key)
                        {
                        case '*': // shift bit 0
                            {
                                keyboard_modifiers &= 0b1110; // not 0100
                                break;
                            }

                        case '^': // ctrl bit 1
                            {
                                keyboard_modifiers &= 0b1101; // not 0010
                                break;
                            }
                        case '!': // alt bit 2
                            {
                                keyboard_modifiers &= 0b1011; // not 0001
                                break;
                            }
                        default:
                            {
                                break;
                            }
                        }
                    }

                    // todo: Add a line buffer and parsing to inputs on enter.
                    // todo: Add an key handler which deals with modifier keys
                    // todo: handle backspace
                    // todo: write an actual terminal class.

                    // WRITE("Key up event in main loop.\n");
                    break;
                }
            case KEY_DOWN:
                {
                    // WRITE("Key down event in main loop: ");
                    const size_t cin = data.lower_data;
                    const char key = key_map[cin];
                    // WRITE(key);
                    // NEWLINE();
                    if (key_map[cin] != 0)
                    {
                        switch (key)
                        {
                        case '\b': // backspace
                            {
                                Terminal::backspace();
                                if (cmd_buffer_idx > 0)
                                {
                                    cmd_buffer[--cmd_buffer_idx] = ' ';
                                }
                                break;
                            }
                        case '\t': // tab
                            {
                                Terminal::write("    ");
                                break;
                            }
                        case '^': // ctrl bit 1
                            {
                                keyboard_modifiers |= 0b0010;
                                break;
                            }
                        case '*': // shift bit 0
                            {
                                keyboard_modifiers |= 0b0001;
                                // print("Shift pressed", keyboard_modifiers);
                                break;
                            }
                        case '!': // alt bit 2
                            {
                                keyboard_modifiers |= 0b0100;
                                break;
                            }
                        case 'H': // Home
                            {
                                // todo: handle home
                                break;
                            }
                        case 'E': // end
                            {
                                // go to end of line
                                break;
                            }
                        case 'U': // up
                            {
                                // probably won't handle up
                                break;
                            }
                        case 'D': // down
                            {
                                // probably won't handle this
                                break;
                            }
                        case '<': // left
                            {
                                // move left
                                break;
                            }
                        case '>': // right
                            {
                                // move right
                                break;
                            }
                        case 'C': // capital C meaning caps lock
                            {
                                keyboard_modifiers ^= 0b1000;
                                break;
                            }
                        case '\n':
                            {
                                Terminal::write("\n");
                                process_cmd();
                                memset(cmd_buffer, 0, cmd_buffer_size);
                                cmd_buffer_idx = 0;
                                Terminal::refresh();
                                break;
                            }
                        default:
                            {
                                bool is_alpha = (key >= 97 && key <= 122);
                                if (keyboard_modifiers & 0b1000) // caps lock enabled
                                    if (is_alpha) // alphanumeric keys get shifted to caps
                                    {
                                        cmd_buffer[cmd_buffer_idx++] = shift_map[cin];
                                        Terminal::write(shift_map[cin]);
                                        break;
                                    }
                                if ((keyboard_modifiers & 0b0001)) // shift is down or capslock is on
                                {
                                    cmd_buffer[cmd_buffer_idx++] = shift_map[cin];
                                    Terminal::write(shift_map[cin]);
                                    break;
                                }
                                else
                                {
                                    cmd_buffer[cmd_buffer_idx++] = key;
                                    Terminal::write(key);
                                }

                                break;
                            }
                        }
                    }
                    break;
                }
            default:
                {
                    auto& term = Terminal::get();
                    term.write("Unhandled event.\n");
                    term.write("Type: ");
                    term.write(static_cast<int>(type));
                    term.write(" lower: ");
                    term.write(data.lower_data, true);
                    term.write(" upper: ");
                    term.write(data.upper_data, true);
                    term.newLine();
                    break;
                }
            }
        }
        // else
        sleep_ms(1);
    }
}

int Shell::process_cmd()
{
    if (cmd_buffer_idx == 0) return -1;
    if (strncasecmp(cmd_buffer, "play doom", 9) == 0)
    {
        Terminal::stop_drawing();
        run_doom();
        Terminal::resume_drawing();
        Terminal::refresh();
    }
    else
    {
        Terminal::write("Unknown command: ", COLOR_RED);
        Terminal::write(cmd_buffer, cmd_buffer_idx, COLOR_MAGENTA);
        Terminal::newLine();
    }
    return 0;
}

