//
// Created by artypoole on 09/09/24.
//

#ifndef CPPMEMORY_H
#define CPPMEMORY_H

#include "types.h"

void *operator new(size_t size);

void *operator new[](size_t size);

void operator delete(void *p);
void operator delete(void *p, size_t);


void operator delete[](void *p);

// inline void *operator new(size_t, void *p)     throw() { return p; }
// inline void *operator new[](size_t, void *p)   throw() { return p; }
// inline void  operator delete  (void *, void *) throw() { };
// inline void  operator delete[](void *, void *) throw() { };


#endif //CPPMEMORY_H
