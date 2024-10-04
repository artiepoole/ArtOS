//
// Created by artypoole on 04/10/24.
//

#ifndef SIMD_H
#define SIMD_H

#include "stddef.h"
#include "stdint.h"
#include "stdbool.h"
#ifdef __cplusplus
extern "C" {
#endif

void simd_enable();
bool simd_enabled();

void simd_copy(void* dest, const void* src, size_t size);


#ifdef __cplusplus
}
#endif
#endif //SIMD_H
