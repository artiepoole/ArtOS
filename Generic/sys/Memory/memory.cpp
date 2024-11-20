#include "memory.h"
#include "multiboot2.h"
#include <logging.h>


extern unsigned char kernel_end;
unsigned char* kernel_brk = &kernel_end;


extern "C"
void* sbrk(const long increment)
{
    void* last_brk = kernel_brk;
    kernel_brk = kernel_brk + increment;
    return last_brk;

    // TODO: this does not work with paging. This needs to be replaced.
}



