//
// Created by artypoole on 09/09/24.
//

#include "CPPMemory.h"

#include "stdlib.h"


void* operator new(size_t size) throw()
{
    if (size == 0) size = 1;

    return malloc(size);
}


void* operator new[](size_t size) throw()
{
    if (size == 0) size = 1;

    return malloc(size);
}


void operator delete(void* p) throw()
{
    if (p) free(p);
}

void operator delete(void* p, size_t) throw()
{
    if (p) free(p);
}

void operator delete[](void* p) throw()
{
    if (p) free(p);
}
