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
    write_standard("DG_init called.\n", 17);
}

extern "C"
void DG_DrawFrame()
{
    VideoGraphicsArray::get().draw_region(DG_ScreenBuffer);
}

extern "C"
void DG_SleepMs(const uint32_t ms)
{
    sleep_ms(ms);
}

extern "C"
uint32_t DG_GetTicksMs()
{
    return get_tick_ms();
}

extern "C"
int DG_GetKey(int* pressed, unsigned char* key)
{
    //todo: get event queue or smth
    return 0;
}

extern "C"
void DG_SetWindowTitle(const char* title)
{

}

extern "C"
int run_doom()
{
    doomgeneric_Create(0, nullptr);

    while (true)
    {
        doomgeneric_Tick();
    }

    return 0;
}


