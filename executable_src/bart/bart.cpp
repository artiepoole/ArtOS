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


void div_0() {
    asm("mov $0, %eax\n"
        "div %eax");
}

BartShell::BartShell() {
    for (char &i: cmd_buffer) {
        i = 0;
    }
}

void BartShell::set_modifier(char key) {
    switch (key) {
        case '*': keyboard_modifiers |= 0b0001;
            break;
        case '^': keyboard_modifiers |= 0b0010;
            break;
        case '!': keyboard_modifiers |= 0b0100;
            break;
        case 'C': keyboard_modifiers ^= 0b1000;
            break;
        default: break;
    }
}

void BartShell::clear_modifier(char key) {
    switch (key) {
        case '*': keyboard_modifiers &= ~0b0001;
            break;
        case '^': keyboard_modifiers &= ~0b0010;
            break;
        case '!': keyboard_modifiers &= ~0b0100;
            break;
        default: break;
    }
}

char BartShell::transform_key(const char key, const size_t cin) const {
    const bool is_alpha = (key >= 'a' && key <= 'z');
    if (keyboard_modifiers & 0b0001 || (keyboard_modifiers & 0b1000 && is_alpha))
        return shift_map[cin];
    return key;
}

void BartShell::key_down(size_t cin) {
    switch (char key = key_map[cin]) {
        case '\0':
            break;
        case '\b': // backspace
        {
            if (cmd_buffer_idx > 0) {
                putchar('\b');
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
            putchar('\n');
            if (cmd_buffer[0] != '\0') {
                process_cmd();
                memset(cmd_buffer, 0, cmd_buffer_size);
                cmd_buffer_idx = 0;
            }
            break;
        }
        default: {
            key = transform_key(key, cin);
            cmd_buffer[cmd_buffer_idx++] = key;
            putchar(key);
            break;
        }
    }
}

int BartShell::process_cmd() {
    printf("%s\n", cmd_buffer);
    if (!strcmp("help\0", cmd_buffer)) {
        printf("Available commands:\n\n"
            "  help        - Show this help message\n\n"
            "  div_0       - Crash immediately (for testing)\n\n"
            "  exit        - Exit the shell (if this is the base shell, kernel "
            "will hang)\n\n"
            "  clear       - Clear the screen\n\n"
            "Available executables (manually populated list):\n\n"
            "  b.art       - (this) Spawns a new shell\n\n"
            "  doom.art    - \\m/  Play DOOM (1993) \\m/ \n\n"
            "  hello.art   - Hello World program\n\n");
        return 0;
    }
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

// TODO: for drawing here, maybe this should have its own screen buffer and should implement the terminal
// stuff itself and then loop "while pending events" and then flush after events are all processed
// TODO: arguments
// TODO: fs commands and shortcuts
// TODO: creating files
[[noreturn]] void BartShell::run() {
    printf("\nb.art started! Need guidance? Try 'help'.\n");
    while (true) {
        bool to_flush = false;
        while (probe_pending_events()) {
            auto [type, data] = get_next_event();
            if (type == KEY_DOWN) {
                const size_t cin = data.lower_data;
                key_down(cin);
                to_flush = true;
            } else if (type == KEY_UP) {
                const size_t cin = data.lower_data;
                const char key = key_map[cin];
                clear_modifier(key);
            } else if (type == NULL_EVENT) {
                printf("NULL EVENT\n");
            } else {
                printf("Unhandled event.\n Type: %x lower: %i upper: %i\n", static_cast<int>(type), data.lower_data,
                       data.upper_data);
                break;
            }
        }
        if (to_flush) {
            fflush(stdout);
        }
    }
}

int main() {
    // Init and load the shell. Shell draws directly to the terminal using printf
    auto shell = BartShell();
    shell.run();
}
