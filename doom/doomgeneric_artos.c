#include "m_argv.h"
#include "doomgeneric.h"
#include "doom.h"


void DG_Init(){

}
void DG_DrawFrame(){

}
void DG_SleepMs(uint32_t ms)
{

}
uint32_t DG_GetTicksMs()
{
    return 0;
}
int DG_GetKey(int* pressed, unsigned char* key)
{
    return 0;
}
void DG_SetWindowTitle(const char * title)
{

}


extern "C"
int run_doom(int argc, char **argv)
{
    int myargc = argc;
    char** myargv = argv;

    M_FindResponseFile();

    DG_ScreenBuffer = malloc(DOOMGENERIC_RESX * DOOMGENERIC_RESY * 4);

    DG_Init();

    D_DoomMain();
}