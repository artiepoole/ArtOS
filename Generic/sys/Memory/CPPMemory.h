//
// Created by artypoole on 09/09/24.
//

#ifndef CPPMEMORY_H
#define CPPMEMORY_H

#include "types.h"

void* operator new(size_t size);

void* operator new[](size_t size);

void operator delete(void* p);
void operator delete(void* p, size_t);


void operator delete[](void* p);


#endif //CPPMEMORY_H
