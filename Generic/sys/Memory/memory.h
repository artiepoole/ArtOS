//
// Created by artypoole on 13/07/24.
//

#ifndef MEMORY_H
#define MEMORY_H

#pragma once

// http://wiki.osdev.org/Memory_Map_(x86)
// "Use the BIOS function INT 15h, EAX=0xE820 to get a reliable map of Extended Memory."
// http://wiki.osdev.org/Detecting_Memory_(x86)#BIOS_Function:_INT_0x15,_EAX_=_0xE820

void* sbrk(long increment);


#endif //MEMORY_H
