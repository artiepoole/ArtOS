//
// Created by artypoole on 04/07/24.
//

#ifndef SYSTEM_H
#define SYSTEM_H

#include "types.h"


struct cpu_registers_t
{
    u32 gs, fs, es, ds; /* pushed the segs last */
    u32 edi, esi, ebp, esp, ebx, edx, ecx, eax; /* pushed by 'pusha' */
    u32 int_no, err_code; /* filled by our 'push byte #' and 'push 0' in IDT.S */
    u32 eip, cs, eflags, useresp, ss; /* pushed by the processor automatically */
}__attribute__((packed));

struct eflags_t
{
    union
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
};

struct cr0_t
{
    union
    {
        struct
        {
            u32 PE : 1; // 0
            u32 MP : 1; // 1
            u32 EM : 1; // 2
            u32 TS : 1; // 3
            u32 ET : 1; // 4
            u32 NE : 1; // 5
            u32 res0 : 10;// up to bit 15
            u32 WP : 1; // 16
            u32 res1 : 1; // 17
            u32 AM : 1; // 18
            u32 res2 : 10;// up to 28
            u32 NW : 1;  // 29
            u32 CD : 1;  // 30
            u32 PG : 1; // 31
        };

        u32 raw;
    };
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
void get_GDT();
u16 get_cs();
u16 get_ds();
cr0_t get_cr0();

#endif //SYSTEM_H
