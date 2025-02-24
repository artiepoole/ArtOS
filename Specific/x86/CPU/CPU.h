// ArtOS - hobby operating system by Artie Poole
// Copyright (C) 2025 Stuart Forbes Poole <artiepoole>
//
//     This program is free software: you can redistribute it and/or modify
//     it under the terms of the GNU General Public License as published by
//     the Free Software Foundation, either version 3 of the License, or
//     (at your option) any later version.
//
//     This program is distributed in the hope that it will be useful,
//     but WITHOUT ANY WARRANTY; without even the implied warranty of
//     MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//     GNU General Public License for more details.
//
//     You should have received a copy of the GNU General Public License
//     along with this program.  If not, see <https://www.gnu.org/licenses/>

//
// Created by artypoole on 04/07/24.
//

#ifndef SYSTEM_H
#define SYSTEM_H

#include "types.h"

// Ring Privilege Level (RPL). 0 is highest (kernel) and 3 is lowest (user)
#define RPL_USER 0x03
#define RPL_KERNEL 0x00

struct cpu_registers_t
{
    // Top of the pushed stack
    u32 gs; //---------------------
    u32 fs; // Pushed by IDT.S last
    u32 es; //
    u32 ds; //---------------------
    u32 edi; //---------------------
    u32 esi; //
    u32 ebp; //
    u32 esp; // Pushed by 'pusha'
    u32 ebx; //
    u32 edx; //
    u32 ecx; //
    u32 eax; //---------------------
    u32 int_no; // Pushed manually
    u32 err_code; // filled by our 'push byte #' and 'push 0' in IDT.S //
    u32 eip; //---------------------
    u32 cs; // pushed by the processor automatically by interrupt
    u32 eflags; //
    u32 user_esp; // useresp and ss are only populated if the ring level changes during the interrupt
    u32 ss; //---------------------
    // Bottom of the pushed stack
}__attribute__((packed));

union eflags_t
{
    struct
    {
        u32 CF : 1;
        u32 res0 : 1;
        u32 PF : 1;
        u32 res1 : 1;
        u32 AF : 1;
        u32 res2 : 1;
        u32 ZF : 1;
        u32 SF : 1;
        u32 TF : 1;
        u32 IF : 1;
        u32 DF : 1;
        u32 OF : 1;
        u32 IOPL : 2;
        u32 NT : 1;
        u32 MD : 1;
        u32 RF : 1;
        u32 VM : 1;
        u32 AC : 1;
        u32 VIF : 1;
        u32 VIP : 1;
        u32 ID : 1;
        u32 res3 : 8;
        u32 AES : 1;
        u32 AI : 1;
    };

    u32 flag_double;
};


union cr0_t
{
    struct
    {
        u32 PE : 1; // 0
        u32 MP : 1; // 1
        u32 EM : 1; // 2
        u32 TS : 1; // 3
        u32 ET : 1; // 4
        u32 NE : 1; // 5
        u32 res0 : 10; // up to bit 15
        u32 WP : 1; // 16
        u32 res1 : 1; // 17
        u32 AM : 1; // 18
        u32 res2 : 10; // up to 28
        u32 NW : 1; // 29
        u32 CD : 1; // 30
        u32 PG : 1; // 31
    };

    u32 raw;
};

eflags_t get_eflags();
bool get_interrupts_are_enabled();

void disable_interrupts();
void enable_interrupts();

void write_register(uintptr_t addr, uintptr_t val);
u32 read_register(uintptr_t addr);
void cpu_get_MSR(u32 msr, u32* lo, u32* hi);
void cpu_set_MSR(u32 msr, u32 lo, u32 hi);


extern u32 DATA_CS;
extern u32 TEXT_CS;
u16 get_cs();
u16 get_ds();
cr0_t get_cr0();
u32 get_cr2();

#endif //SYSTEM_H
