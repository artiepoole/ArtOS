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


#include "CPUID.h"

static void enable_sse()
{
    uint32_t cr4;
    __asm__ __volatile__ ("mov %%cr4, %0" : "=r" (cr4));
    // Set OSFXSR and OSXMMEXCPT (bits 9 and 10 respectively)
    cr4 |= 3 << 9;
    __asm__ __volatile__ ("mov %0, %%cr4" : : "r" (cr4));
}

/* https://wiki.osdev.org/SSE */
void simd_enable()
{
    cpuid_feature_info_t const* info = cpuid_get_feature_info();
    if (!(info->edx & 0x1 << 24))
    {
        while (true)
        {
            //empty
        }
    }
    if (info->edx & 0x1 << 26)
    {
        enable_sse();
    }
    // AVX256 is ecx & 0x1<<28
}
