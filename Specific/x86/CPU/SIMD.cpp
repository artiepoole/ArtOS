//
// Created by artypoole on 04/10/24.
//

#include "SIMD.h"
#include "CPUID.h"

bool simd_is_enabled = false;


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


static void enable_sse()
{
    uint32_t cr4;
    __asm__ __volatile__ ("mov %%cr4, %0" : "=r" (cr4));
    // Set OSFXSR and OSXMMEXCPT (bits 9 and 10 respectively)
    cr4 |= 3 << 9;
    __asm__ __volatile__ ("mov %0, %%cr4" : : "r" (cr4));
}

/* https://wiki.osdev.org/SSE */
extern "C"
void simd_enable()
{
    cpuid_feature_info_t const* info = cpuid_get_feature_info();
    if (!info->edx & 0x1 << 24)
    {
        return;
    }
    if (info->edx & 0x1 << 26)
    {
        enable_sse();
        simd_is_enabled = true;
    }
    // AVX256 is ecx & 0x1<<28
}


bool simd_enabled()
{
    return simd_is_enabled;
}


extern "C"
void* simd_copy(void* dest, const void* src, const size_t size)
{
    auto* d = static_cast<unsigned char*>(dest);
    auto* s = static_cast<const unsigned char*>(src);

    // Copy in 16-byte chunks
    size_t i = 0;
    for (; i + sizeof(m128i) < size; i += sizeof(m128i))
    {
        const m128i chunk = mm_loadu_si128(reinterpret_cast<const m128i*>(s + i)); // Load 16 bytes
        mm_storeu_si128(reinterpret_cast<m128i*>(d + i), chunk); // Store 16 bytes
    }

    // Copy any remaining bytes
    for (; i < size; i++)
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
