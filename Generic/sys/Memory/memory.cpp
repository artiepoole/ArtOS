#include "memory.h"
#include "multiboot2.h"
#include <logging.h>


extern unsigned char kernel_end;
unsigned char* kernel_brk = &kernel_end;


extern "C"
void* sbrk(const long increment)
{
    const auto last_brk = reinterpret_cast<uintptr_t>(kernel_brk);
    kernel_brk = kernel_brk + increment;
    void* retval = mmap(last_brk, increment, false, false, false, 0);
    return retval;
}



