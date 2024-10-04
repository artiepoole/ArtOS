//
// Created by artypoole on 04/10/24.
//

#include "SIMD.h"

extern "C"
void simd_copy(void* dest, const void* src, size_t size) {
    const size_t simd_width = sizeof(__m128i); // SIMD register width (16 bytes)
    const size_t simd_iters = size / simd_width; // How many full SIMD registers to copy
    const size_t remainder = size % simd_width; // How many bytes are left to copy normally

    auto* simd_src = static_cast<const __m128i*>(src);
    auto* simd_dest = static_cast<__m128i*>(dest);

    // Copy 16 bytes at a time using SIMD
    for (size_t i = 0; i < simd_iters; i++) {
        const __m128i data = _mm_loadu_si128(&simd_src[i]);  // Load 16 bytes from source
        _mm_storeu_si128(&simd_dest[i], data);         // Store 16 bytes to destination
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