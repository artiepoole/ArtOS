#include <EventQueue.h>

#include "../include/constants/types.h"

extern "C" {
#include "doomgeneric/doomgeneric.h"
}

#include "../src/sys/kernel.h"
#include "../include/VideoGraphicsArray.h"

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
    auto & queue = EventQueue::getInstance();
    if (!queue.pendingEvents()) return 0; // no events

    auto latest = queue.getEvent();
    switch (latest.type)
    {
        case KEY_DOWN:
            *pressed = 1;
            *key = latest.data.lower_data;
            break;
        case KEY_UP:
            *pressed = 0;
            *key = latest.data.lower_data;
            break;
    }
    return queue.pendingEvents();

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
