//
// Created by artypoole on 18/07/24.
//

#ifndef CPUID_H
#define CPUID_H
#include "types.h"

// https://en.wikipedia.org/wiki/CPUID#Calling_CPUID

struct cpuid_manufacturer_info_t
{
    u32 max_param;

    union
    {
        struct
        {
            u32 ebx;
            u32 edx;
            u32 ecx;
        };

        char manufacturer_ID[12];
    };
};

struct cpuid_ext_manufacturer_info_t
{
    u32 max_ext_param;

};

union cpuid_feature_info_t
{
    struct
    {
        u32 eax;
        u32 ebx;
        u32 ecx;
        u32 edx;
    };

    u32 raw[4];
};

struct cpuid_core_frequency_info_t
{
    u32 tsc_ratio_denom;
    u32 tsc_ratio_numer;
    u32 core_clock_freq_hz;
    u32 cpu_base_freq_MHz;
    u32 cpu_max_freq_MHz;
    u32 bus_ref_freq_MHz;
    u32 tsc_freq;
};

cpuid_manufacturer_info_t *cpuid_get_manufacturer_info(); // leaf 0
cpuid_ext_manufacturer_info_t* cpuid_print_ext_manufacturer_info(); //leaf 0x8000000

cpuid_feature_info_t* cpuid_get_feature_info(); // leaf 1

cpuid_core_frequency_info_t* cpuid_get_frequency_info(); // leaf 0x15/0x16


//todo: write a total reading of CPUID leaves to make it easier to access data by the common naming scheme:  CPUID.**h:E*X[bit *]


#endif //CPUID_H
