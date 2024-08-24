//
// Created by artypoole on 18/07/24.
//

#include "TSC.h"

u64 TSC_get_ticks()
{
    u32 eax, edx;
    asm volatile("rdtsc": "=a" (eax),"=d" (edx));
    return static_cast<u64>(eax) | static_cast<u64>(edx) << 32;
}
