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


#include "stdlib.h"

#include "doomtype.h"
#include "i_system.h"

#include "m_fixed.h"


// Fixme. __USE_C_FIXED__ or something.

fixed_t
FixedMul
(fixed_t a,
 fixed_t b)
{
    return ((int64_t)a * (int64_t)b) >> FRACBITS;
}


//
// FixedDiv, C version.
//

fixed_t FixedDiv(fixed_t a, fixed_t b)
{
    if ((abs(a) >> 14) >= abs(b))
    {
        return (a ^ b) < 0 ? INT_MIN : INT_MAX;
    }
    else
    {
        int64_t result;

        result = ((int64_t)a << 16) / b;

        return (fixed_t)result;
    }
}
