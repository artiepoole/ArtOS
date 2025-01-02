#include "memory.h"
#include "multiboot2.h"
#include <logging.h>

#include "stdlib.h"


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

void* aligned_malloc(size_t size, size_t alignment)
{
    // Allocate extra space to account for alignment and for storing the original pointer
    void* ptr = malloc(size + alignment - 1 + sizeof(void*));
    if (ptr == NULL)
    {
        return NULL; // Allocation failed
    }

    // Calculate the aligned memory address
    uintptr_t addr = (uintptr_t)ptr;
    uintptr_t aligned_addr = (addr + alignment - 1 + sizeof(void*)) & ~(alignment - 1);

    // Store the original pointer just before the aligned memory
    void** aligned_ptr = (void**)(aligned_addr - sizeof(void*));
    *aligned_ptr = ptr;

    return (void*)aligned_addr;
}

void aligned_free(void* ptr)
{
    // Retrieve the original pointer and free the allocated memory
    if (ptr != NULL)
    {
        void** aligned_ptr = (void**)((uintptr_t)ptr - sizeof(void*));
        free(*aligned_ptr);
    }
}
