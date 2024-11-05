//
// Created by artypoole on 27/06/24.
//

#ifndef SPLASH_SCREEN_H
#define SPLASH_SCREEN_H


#include "types.h"
inline u32 load_bar_region[4] = {248, 483, 534, 24}; // TODO: Calculate this instead.
inline u32 splash_logo_width = 540;
inline u32 splash_logo_height = 160;
extern u32 SPLASH_DATA[540 * 160];
#endif //SPLASH_SCREEN_H
