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
// Created by artiepoole on 5/3/25.
//

#include "bart.h"

#include <stdlib.h>

#include "event.h"
#include "kernel.h"
#include "keymaps/key_maps.h"
#include "stdio.h"
#include "string.h"

BartShell::BartShell() {
    for (char &i: cmd_buffer) {
        i = 0;
    }
}


// TODO: for drawing here, maybe this should have its own screen buffer and should implement the terminal
// stuff itself and then loop "while pending events" and then flush after events are all processed
// TODO: arguments
// TODO: fs commands and shortcuts
// TODO: creating files
[[noreturn]] void BartShell::run() {
    printf("Shell started\n");
    while (true) {
        while (probe_pending_events()) {
            switch (auto [type, data] = get_next_event(); type) {
                case NULL_EVENT: {
                    printf("NULL EVENT\n");
                    break;
                }
                case KEY_UP: {
                    const size_t cin = data.lower_data;
                    const char key = key_map[cin];

                    if (key_map[cin] != 0) {
                        switch (key) {
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
                            default: {
                                break;
                            }
                        }
                    }
                    break;
                }
                case KEY_DOWN: {
                    const size_t cin = data.lower_data;
                    const char key = key_map[cin];
                    if (key_map[cin] != 0) {
                        switch (key) {
                            case '\b': // backspace
                            {
                                printf("%c", '\b');
                                if (cmd_buffer_idx > 0) {
                                    cmd_buffer[--cmd_buffer_idx] = '\0';
                                }
                                break;
                            }
                            case '\t': // tab
                            {
                                printf("    ");
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
                            case '\n': {
                                printf("\n");
                                if (cmd_buffer[0] == '\0') { break; }
                                process_cmd();
                                memset(cmd_buffer, 0, cmd_buffer_size);
                                cmd_buffer_idx = 0;
                                // get_terminal().refresh();
                                break;
                            }
                            default: {
                                const bool is_alpha = (key >= 97 && key <= 122);
                                if (keyboard_modifiers & 0b1000) // caps lock enabled
                                    if (is_alpha) // alphanumeric keys get shifted to caps
                                    {
                                        cmd_buffer[cmd_buffer_idx++] = shift_map[cin];
                                        printf("%c", shift_map[cin]);
                                        break;
                                    }
                                if ((keyboard_modifiers &
                                     0b0001)) // shift is down or capslock is on
                                {
                                    cmd_buffer[cmd_buffer_idx++] = shift_map[cin];
                                    printf("%c", shift_map[cin]);
                                    break;
                                }
                                cmd_buffer[cmd_buffer_idx++] = key;
                                printf("%c", key);
                                break;
                            }
                        }
                    }
                    break;
                }
                default: {
                    printf("Unhandled event.\n Type: %x lower: %i upper: %i\n",
                           static_cast<int>(type), data.lower_data, data.upper_data);
                    break;
                }
            }
        }
        fflush(stdout);
    }
}

void div_0() {
    asm("mov $0, %eax\n"
        "div %eax");
}

int BartShell::process_cmd() {
    printf("%s\n", cmd_buffer);
    if (!strcmp("exit\0", cmd_buffer)) {
        exit(0);
    }
    if (!strcmp("clear\0", cmd_buffer)) {
        clear_term();
        return 0;
    }
    if (!strcmp("div_0\0", cmd_buffer)) {
        div_0();
        return 0;
    }
    FILE *f = fopen(cmd_buffer, "rb");
    if (f->handle > 0) {
        printf("executing %s\n", cmd_buffer);
        const int ret = execf(f->handle);
        printf("%s exited with exit code %d\n", cmd_buffer, ret);
        return ret;
    }
    printf("File not found: %s\n", cmd_buffer);
    return 0;
}

int main() {
    // Init and load the shell. Shell draws directly to the terminal using printf
    auto shell = BartShell();
    shell.run();
}
