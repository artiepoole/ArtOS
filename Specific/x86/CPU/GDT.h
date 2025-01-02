//
// Created by artypoole on 25/11/24.
//

#ifndef GDT_H
#define GDT_H

constexpr size_t null_offset = 0x0;
constexpr size_t kernel_cs_offset = 0x8;
constexpr size_t kernel_ds_offset = 0x10;
constexpr size_t user_cs_offset = 0x18;
constexpr size_t user_ds_offset = 0x20;
constexpr size_t tss_offset = 0x28;

void GDT_init();


#endif //GDT_H
