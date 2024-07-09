#include <stdint.h>

extern char kernel_end;
void* kernel_brk = &kernel_end;

extern "C"
void * sbrk( intptr_t increment ) {
    void* last_brk = kernel_brk;
    kernel_brk = kernel_brk + increment;
    return last_brk;
}