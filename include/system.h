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
            bool CF : 1;
            bool res0 : 1;
            bool PF : 1;
            bool res1 : 1;
            bool AF : 1;
            bool res2 : 1;
            bool ZF : 1;
            bool SF : 1;
            bool TF : 1;
            bool IF : 1;
            bool DF : 1;
            bool OF : 1;
            u8 IOPL : 2;
            bool NT : 1;
            bool MD : 1;
            bool RF : 1;
            bool VM : 1;
            bool AC : 1;
            bool VIF : 1;
            bool VIP : 1;
            bool ID : 1;
            u8 res3 : 8;
            bool AES : 1;
            bool AI : 1;
        };

        u32 flag_double;
    };
};

eflags_t get_eflags();

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


#endif //SYSTEM_H
