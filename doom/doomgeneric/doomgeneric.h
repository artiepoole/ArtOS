#ifndef DOOM_GENERIC
#define DOOM_GENERIC

#include "stdlib.h"
#include <stdint.h>
#include "d_main.h"

#ifndef DOOMGENERIC_RESX
#define DOOMGENERIC_RESX 1024
#endif  // DOOMGENERIC_RESX

#ifndef DOOMGENERIC_RESY
#define DOOMGENERIC_RESY 768
#endif  // DOOMGENERIC_RESY

#include "doomtypes.h"
#ifdef CMAP256

typedef uint8_t pixel_t;

#else  // CMAP256

typedef u32 pixel_t;

#endif  // CMAP256


extern pixel_t* DG_ScreenBuffer;

void doomgeneric_Create(int argc, char **argv);
void doomgeneric_Tick();


//Implement below functions for your platform

void DG_Init();
void DG_DrawFrame();
void DG_SleepMs(uint32_t ms);
uint32_t DG_GetTicksMs();
int DG_GetKey(int* pressed, unsigned char* key);
void DG_SetWindowTitle(const char * title);


int run_doom();

#endif //DOOM_GENERIC
