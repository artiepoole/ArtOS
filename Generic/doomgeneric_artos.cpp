#include "EventQueue.h"
#include "ports.h"

#include "../doom/doomgeneric/doomkeys.h"
#include "types.h"

#include "sys/kernel.h"

extern "C" {
#include "../doom/doomgeneric/doomgeneric.h"


u8 doom_key_map[128] =
{
    0, KEY_ESCAPE, '1', '2', '3', '4', '5', '6', '7', '8', /* 9 */
    '9', '0', '-', '=', KEY_BACKSPACE, /* Backspace */
    KEY_TAB, /* Tab */
    'q', KEY_UPARROW, KEY_USE, 'r', /* 19 */ // q w e r
    't', 'y', 'u', 'i', 'o', 'p', '[', ']', KEY_ENTER, /* Enter key */
    KEYB_CONTROL, /* 29   - Control */
    KEY_STRAFE_L, KEY_DOWNARROW, KEY_STRAFE_R, 'f', 'g', 'h', KEY_LEFTARROW, KEY_FIRE, KEY_RIGHTARROW, ';', /* 39 */
    '\'', '`', KEY_RSHIFT, /* Left shift */
    '\\', 'z', 'x', 'c', 'v', 'b', 'n', /* 49 */
    'm', ',', '.', '/', KEY_RSHIFT, /* Right shift */
    '*',
    KEY_LALT, /* Alt */
    ' ', /* Space bar */
    KEY_CAPSLOCK, /* Caps lock */
    KEY_F1, /* 59 - F1 key ... > */
    KEY_F2, KEY_F3, KEY_F4, KEY_F5, KEY_F6, KEY_F7, KEY_F8, KEY_F9,
    KEY_F10, /* < ... F10 */
    0, /* 69 - Num lock*/
    0, /* Scroll Lock */
    KEY_HOME, /* Home key */
    KEY_UPARROW, /* Up Arrow */
    0, /* Page Up */
    KEY_MINUS,
    KEY_LEFTARROW, /* Left Arrow */
    0,
    KEY_RIGHTARROW, /* Right Arrow */
    KEYP_PLUS,
    KEY_END, /* 79 - End key*/
    KEY_DOWNARROW, /* Down Arrow */
    KEY_PGDN, /* Page Down */
    KEY_INS, /* Insert Key */
    KEY_DEL, /* Delete Key */
    0, 0, 0,
    KEY_F11, /* F11 Key */
    KEY_F12, /* F12 Key */
    0, /* All other keys are undefined */
};


void DG_Init()
{
    // Nothing to do here but maybe this should prep the FS if not init.
}

void DG_DrawFrame()
{
    draw_screen_region(DG_ScreenBuffer);
}

void DG_SleepMs(const uint32_t ms)
{
    sleep_ms(ms);
}

uint32_t DG_GetTicksMs()
{
    return get_tick_ms();
}

int DG_GetKey(int* pressed, unsigned char* key)
{
    // should return 0 if no events
    // should update just one keypress if keypress
    // should skip to next event if not keypress

    // TODO: implement syscall for get events.
    auto& queue = EventQueue::getInstance();
    if (!queue.pendingEvents()) return 0; // no events


    switch (const auto latest = queue.getEvent(); latest.type)
    {
    case KEY_DOWN:
        *pressed = 1;
        *key = doom_key_map[latest.data.lower_data];
        break;
    case KEY_UP:
        *pressed = 0;
        *key = doom_key_map[latest.data.lower_data];
        break;
    default: break;
    }
    return 1;
}

void DG_SetWindowTitle(const char*)
{
    // No window to set title on.
}

int run_doom()
{
    doomgeneric_Create(0, nullptr);

    while (doomgeneric_Tick());


    return 0;
}

/* I have not implemented generic handling of functions as executable files yet, so I needed to create a different entry point for doom. */
void run_doom_noret()
{
    doomgeneric_Create(0, nullptr);

    while (doomgeneric_Tick());

    exit(0); // should be unreachable.
}
}
