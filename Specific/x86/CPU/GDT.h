//
// Created by artypoole on 25/11/24.
//

#ifndef GDT_H
#define GDT_H

constexpr u16 null_offset = 0x0;
constexpr u16 kernel_cs_offset = 0x8;
constexpr u16 kernel_ds_offset = 0x10;
constexpr u16 user_cs_offset = 0x18;
constexpr u16 user_ds_offset = 0x20;
constexpr u16 tss_offset = 0x28;

void GDT_init();


#endif //GDT_H
