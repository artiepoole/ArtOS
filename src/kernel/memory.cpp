#include <stdint.h>

extern char kernel_end;
extern void* kernel_brk;

extern "C"
void * sbrk( intptr_t increment ) {
    void* last_brk = kernel_brk;
    kernel_brk = kernel_brk + increment;
    return last_brk;
}