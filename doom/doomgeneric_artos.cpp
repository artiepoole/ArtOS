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
    // fprintf("DG_GetKey called: \n");
    //todo: get event queue or smth
    return 0;
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


