//
// Created by artypoole on 04/10/24.
//

#include "SIMD.h"
#include "CPUID.h"

bool simd_is_enabled = false;
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


static void enable_sse()
{
    uint32_t cr4;
    __asm__ __volatile__ ("mov %%cr4, %0" : "=r" (cr4));
    // Set OSFXSR (bit 9) and OSXMMEXCPT (bit 10)
    cr4 |= 3 << 9;
    __asm__ __volatile__ ("mov %0, %%cr4" : : "r" (cr4));
}

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
        simd_is_enabled = true;
        enable_sse();
    }
}


extern "C"
void simd_copy(void* dest, const void* src, size_t size) {
    constexpr size_t simd_width = sizeof(m128i); // SIMD register width (16 bytes)
    const size_t simd_iters = size / simd_width; // How many full SIMD registers to copy
    const size_t remainder = size % simd_width; // How many bytes are left to copy normally

    auto* simd_src = static_cast<const m128i*>(src);
    auto* simd_dest = static_cast<m128i*>(dest);

    // Copy 16 bytes at a time using SIMD
    for (size_t i = 0; i < simd_iters; i++) {
        const m128i data = mm_loadu_si128(&simd_src[i]); // Load 16 bytes from source
        mm_storeu_si128(&simd_dest[i], data); // Store 16 bytes to destination
    }

    // Copy the remaining bytes normally
    if (remainder > 0) {
        const char* byte_src = static_cast<const char*>(src) + simd_iters * simd_width;
        char* byte_dest = static_cast<char*>(dest) + simd_iters * simd_width;
        for (size_t i = 0; i < remainder; i++) {
            byte_dest[i] = byte_src[i];
        }
    }
}


bool simd_enabled()
{
    return simd_is_enabled;
}
