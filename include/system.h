//
// Created by artypoole on 04/07/24.
//

#ifndef SYSTEM_H
#define SYSTEM_H

#include "types.h"


struct registers
{
    u32 gs, fs, es, ds; /* pushed the segs last */
    u32 edi, esi, ebp, esp, ebx, edx, ecx, eax; /* pushed by 'pusha' */
    u32 int_no, err_code; /* our 'push byte #' and ecodes do this */
    u32 eip, cs, eflags, useresp, ss; /* pushed by the processor automatically */
}__attribute__((packed));




#endif //SYSTEM_H
