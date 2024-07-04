//
// Created by artypoole on 04/07/24.
//

#ifndef SYSTEM_H
#define SYSTEM_H

#include "types.h"
#include <stdint.h>



struct registers
{
    // unsigned int gs, fs, es, ds; /* pushed the segs last */
    // unsigned int edi, esi, ebp, esp, ebx, edx, ecx, eax; /* pushed by 'pusha' */
    unsigned int int_no, err_code; /* our 'push byte #' and ecodes do this */
    unsigned int eip, cs, eflags, useresp, ss; /* pushed by the processor automatically */
};



#endif //SYSTEM_H
