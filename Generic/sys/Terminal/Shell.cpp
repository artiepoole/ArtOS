// ArtOS - hobby operating system by Artie Poole
// Copyright (C) 2025 Stuart Forbes Poole <artiepoole>
//
//     This program is free software: you can redistribute it and/or modify
//     it under the terms of the GNU General Public License as published by
//     the Free Software Foundation, either version 3 of the License, or
//     (at your option) any later version.
//
//     This program is distributed in the hope that it will be useful,
//     but WITHOUT ANY WARRANTY; without even the implied warranty of
//     MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//     GNU General Public License for more details.
//
//     You should have received a copy of the GNU General Public License
//     along with this program.  If not, see <https://www.gnu.org/licenses/>

//
// Created by artypoole on 22/11/24.
//

#include "Shell.h"

#include <ELF.h>
#include <Files.h>
#include <syscall.h>
#include <logging.h>

#include "Scheduler.h"

extern "C" {
#include "doomgeneric/doomgeneric.h"
}

#include "Terminal.h"
#include "EventQueue.h"


Shell::Shell()
{
    for (char& i : cmd_buffer)
    {
        i = 0;
    }
}

void Shell::run()
{
    Terminal::get().write("Shell started\n");
    while (true)
    {
        if (kprobe_pending_events())
        {
            auto [type, data] = kget_next_event();
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
        // // else
        // kpause_exec(0);
    }
}

void div_0()
{
    asm (
        "mov $0, %eax\n"
        "div %eax"
    );
}

void user_test()
{
    // This cannot be loaded in userspace because the memory required to access this function is statically linked in physical memory in the kernel space. You HAVE to load from file or copy memory regions in order to execute in user space.
    LOG("User space should not work.");
}

int Shell::process_cmd()
{
    // TODO: implement actual command lookup of executables.
    if (cmd_buffer_idx == 0) return -1;
    if (strncasecmp(cmd_buffer, "play doom", 10) == 0)
    {
        Terminal::stop_drawing();
        Scheduler::execf(run_doom_noret, "doom");
        Terminal::resume_drawing();
        Terminal::refresh();
    }
    else if (strncasecmp(cmd_buffer, "div0", 5) == 0)
    {
        Terminal::stop_drawing();
        Scheduler::execf(div_0, "div0");
        Terminal::resume_drawing();
        Terminal::refresh();
    }
    else if (strncasecmp(cmd_buffer, "test", 5) == 0)
    {
        Terminal::stop_drawing();
        Scheduler::execf(user_test, "test", true);
        Terminal::resume_drawing();
        Terminal::refresh();
    }
    else if (strncasecmp(cmd_buffer, "readelf", 8) == 0)
    {
        ELF_header_t elf_header{};
        const auto fid = art_open("hello.elf", 0);
        art_read(fid, reinterpret_cast<char*>(&elf_header), sizeof(ELF_header_t));
        art_exec(fid);
        LOG("ELF header read: ", elf_header.e_ident.magic);
    }
    else
    {
        Terminal::write("Unknown command: ", COLOR_RED);
        Terminal::write(cmd_buffer, cmd_buffer_idx, COLOR_MAGENTA);
        Terminal::newLine();
    }
    return 0;
}
