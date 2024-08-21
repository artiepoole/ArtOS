#include <EventQueue.h>
#include <logging.h>
#include <ports.h>

#include "doomgeneric/doomkeys.h"
#include "../include/constants/types.h"

extern "C" {
#include "doomgeneric/doomgeneric.h"
}


#include "../src/sys/kernel.h"
#include "../include/VideoGraphicsArray.h"


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

u32 screen_buffer[DOOMGENERIC_RESX * DOOMGENERIC_RESY];

extern "C"
void DG_Init()
{

}

extern "C"
void DG_DrawFrame()
{
    VideoGraphicsArray::get().draw_region(DG_ScreenBuffer);
}

extern "C"
void DG_SleepMs(const uint32_t ms)
{
    // fprintf("DG_SleepMs called: %d\n", ms);
    LOG("Sleeping for ", ms, " ms");
    sleep_ms(ms);
}

extern "C"
uint32_t DG_GetTicksMs()
{
    // fprintf("DG_GetTicksMs called: \n");
    return get_tick_ms();
}

extern "C"
int DG_GetKey(int* pressed, unsigned char* key)
{
    // should return 0 if no events
    // should update just one keypress if keypress
    // should skip to next event if not keypress
    // TODO: Sending repeatedly held keys not working as intended. Keys not getting detected between frames
    auto& queue = EventQueue::getInstance();
    if (!queue.pendingEvents()) return 0; // no events

    auto latest = queue.getEvent();

    switch (latest.type)
    {
    case KEY_DOWN:
        *pressed = 1;
        *key = doom_key_map[latest.data.lower_data];
        break;
    case KEY_UP:
        *pressed = 0;
        *key = doom_key_map[latest.data.lower_data];
        break;
    }
    return 1;
}

extern "C"
void DG_SetWindowTitle(const char* title)
{
    // fprintf("DG_SetWindowTitle called: \n");
}

extern "C"
int run_doom()
{
    // printf("run_doom called: \n");
    doomgeneric_Create(0, nullptr);

    while (true)
    {
        doomgeneric_Tick();
    }

    return 0;
}
