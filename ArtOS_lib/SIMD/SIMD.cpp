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

//
// Created by artypoole on 04/10/24.
//

#include "_types.h"
#include "SIMD.h"

typedef char v16qi __attribute__ ((__vector_size__ (16)));

// Copied from compiler emmintrin.h
typedef long long m128i __attribute__ ((__vector_size__ (16), __may_alias__));
typedef double m128d __attribute__ ((__vector_size__ (16), __may_alias__));

/* Unaligned version of the same types.  */
typedef long long m128i_u __attribute__ ((__vector_size__ (16), __may_alias__, __aligned__ (1)));
typedef double m128d_u __attribute__ ((__vector_size__ (16), __may_alias__, __aligned__ (1)));

extern __inline m128i __attribute__((__gnu_inline__, __always_inline__, __artificial__))
mm_loadu_si128(m128i_u const* P)
{
    return *P;
}

extern __inline void __attribute__((__gnu_inline__, __always_inline__, __artificial__))
mm_storeu_si128(m128i_u* P, const m128i B)
{
    *P = B;
}

extern __inline m128i __attribute__((__gnu_inline__, __always_inline__, __artificial__))
mm_set_epi8(const char q15, const char q14, const char q13, const char q12,
            const char q11, const char q10, const char q09, const char q08,
            const char q07, const char q06, const char q05, const char q04,
            const char q03, const char q02, const char q01, const char q00)
{
    return __extension__ reinterpret_cast<m128i>((v16qi){
        q00, q01, q02, q03, q04, q05, q06, q07,
        q08, q09, q10, q11, q12, q13, q14, q15
    });
}

extern __inline m128i __attribute__((__gnu_inline__, __always_inline__, __artificial__))
mm_set1_epi8(const char A)
{
    return mm_set_epi8(A, A, A, A, A, A, A, A,
                       A, A, A, A, A, A, A, A);
}

// TODO: does this start need to be aligned?
extern "C"
void* simd_copy(void* dest, const void* src, u32 size)
{
    // auto* d = static_cast<unsigned char*>(dest);
    // auto* s = static_cast<const unsigned char*>(src);
    //
    // // Copy in 16-byte chunks
    // size_t i = 0;
    // for (; i + sizeof(m128i) < size; i += sizeof(m128i))
    // {
    //     const m128i chunk = mm_loadu_si128(reinterpret_cast<const m128i*>(s + i)); // Load 16 bytes
    //     mm_storeu_si128(reinterpret_cast<m128i*>(d + i), chunk); // Store 16 bytes
    // }
    //
    // // Copy any remaining bytes
    // for (; i < size; i++)
    // {
    //     d[i] = s[i];
    // }
    //
    // return dest;
    auto* d = static_cast<unsigned char*>(dest);
    auto* s = static_cast<const unsigned char*>(src);

    // Alignment boundaries
    uintptr_t dest_align_offset = reinterpret_cast<uintptr_t>(d) % 16;

    // Handle unaligned head
    size_t head_bytes = (16 - dest_align_offset) % 16;
    if (head_bytes > size)
    {
        head_bytes = size; // Cap head processing to remaining size
    }
    for (size_t i = 0; i < head_bytes; i++)
    {
        d[i] = s[i];
    }

    d += head_bytes;
    s += head_bytes;
    size -= head_bytes;

    // Handle aligned main block
    size_t simd_chunks = size / 16;
    for (size_t i = 0; i < simd_chunks; i++)
    {
        m128i chunk = mm_loadu_si128((const m128i*)s); // Unaligned load
        mm_storeu_si128((m128i*)d, chunk); // Aligned store
        d += 16;
        s += 16;
    }

    size_t tail_bytes = size % 16;

    // Handle remaining unaligned tail
    for (size_t i = 0; i < tail_bytes; i++)
    {
        d[i] = s[i];
    }
    return dest;
}


extern "C"
void* simd_move(void* dest, const void* src, const size_t size)
{
    auto* d = static_cast<unsigned char*>(dest);
    auto* s = static_cast<const unsigned char*>(src);

    if (d == s || size == 0)
    {
        return dest;
    }
    if (d < s || d >= s + size)
    {
        // Non-overlapping regions, copy forward
        size_t i = 0;
        for (; i + sizeof(m128i) < size; i += sizeof(m128i))
        {
            const m128i chunk = mm_loadu_si128((const m128i*)(s + i)); // Load 16 bytes
            mm_storeu_si128(reinterpret_cast<m128i*>(d + i), chunk); // Store 16 bytes
        }
        // Copy any remaining bytes
        for (; i < size; i++)
        {
            d[i] = s[i];
        }
    }
    else
    {
        // Overlapping regions, copy backward
        size_t i = size;
        while (i >= sizeof(m128i))
        {
            i -= sizeof(m128i);
            const m128i chunk = mm_loadu_si128(reinterpret_cast<const m128i*>(s + i));
            mm_storeu_si128(reinterpret_cast<m128i*>(d + i), chunk);
        }
        // Copy any remaining bytes
        while (i > 0)
        {
            i--;
            d[i] = s[i];
        }
    }

    return dest;
}

void* simd_set(void* dest, const int value, const size_t size)
{
    auto* d = static_cast<char*>(dest);
    const auto val = static_cast<const char>(value);

    // Create a 128-bit value that repeats the byte `val`
    const m128i simd_val = mm_set1_epi8(val);

    size_t i = 0;
    // Set memory in 16-byte chunks using SIMD
    for (; i + sizeof(m128i) < size; i += sizeof(m128i))
    {
        mm_storeu_si128(reinterpret_cast<m128i*>(d + i), simd_val);
    }

    // Set any remaining bytes
    for (; i < size; i++)
    {
        d[i] = val;
    }

    return dest;
}
