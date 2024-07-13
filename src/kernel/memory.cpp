#include <stdint.h>

extern unsigned char kernel_end;
unsigned char * kernel_brk = &kernel_end;

extern "C"
void* sbrk(const long increment)
{
    void* last_brk = kernel_brk;
    kernel_brk = kernel_brk + increment;
    return last_brk;
}
