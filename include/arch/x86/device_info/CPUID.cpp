//
// Created by artypoole on 18/07/24.
//

#include "CPUID.h"


#include <Terminal.h>
#include "types.h"
#include "logging.h"

u8 binaryNum[32];
u8 decimal[10] = {'0', '1', '2', '3', '4', '5', '6', '7', '8', '9'};
u32 max_param = 0;
u32 max_extended_param = 0;

void decToBinary(u32 n)
{
    // counter for binary array
    int i = 0;
    while (n > 0)
    {
        // storing remainder in binary array
        binaryNum[i] = n % 2;
        n = n / 2;
        i++;
    }
}


// In general, move a requested leaf into eax
// call cpuid
// read registers out into variables
// parse the info.

//todo: put this data into structs for easier inspection for each leaf.

void cpuid_print_manufacturer_info()
{

    u32 info[3];
    asm volatile("mov $0x0, %eax\n\t");
    asm volatile("cpuid\n\t":"=a"(max_param), "=b" (info[0]), "=d" (info[1]), "=c" (info[2]));
    // __asm__("mov %%eax, %0\n\t":"=a" (max_param));
    // __asm__("mov %%ebx, %0\n\t":"=r" (info[0]));
    // __asm__("mov %%edx, %0\n\t":"=r" (info[1]));
    // __asm__("mov %%ecx, %0\n\t":"=r" (info[2]));

    WRITE("CPUID Manufacturer ID: ");
    for (size_t i = 0; i < 3; i++)
    {
        for (size_t c = 0; c < 32 / 8; c++)
        {
            WRITE(static_cast<char>(info[i] >> (c * 8)));
        }
    }
    NEWLINE();

    asm volatile("mov $0x80000000, %eax\n\t");
    asm volatile("cpuid\n\t":"=a"(max_extended_param));

    // brand string
    if (max_param - 0x80000000 >= 4)
    {
        u32 parts[12];
        asm volatile("mov $0x80000002, %eax\n\t");
        asm volatile("cpuid\n\t":"=a"(parts[0]), "=b" (parts[1]), "=c" (parts[2]), "=d" (parts[3]));
        asm volatile("mov $0x80000003, %eax\n\t");
        asm volatile("cpuid\n\t":"=a"(parts[4]), "=b" (parts[5]), "=c" (parts[6]), "=d" (parts[7]));
        asm volatile("mov $0x80000004, %eax\n\t");
        asm volatile("cpuid\n\t":"=a"(parts[8]), "=b" (parts[9]), "=c" (parts[10]), "=d" (parts[11]));
        WRITE("Brand string: ");
        for (size_t i = 0; i < 12; i++)
        {
            for (size_t c = 0; c < 32 / 8; c++)
            {
                char letter = static_cast<char>(parts[i] >> (c * 8));
                if(letter ==0){break;}
                WRITE(letter);
            }
        }
        NEWLINE();
    }

    WRITE("CPUID max param: ");
    WRITE(max_param, true);
    NEWLINE();
    WRITE("CPUID max extended param: ");
    WRITE(max_extended_param, true);
    NEWLINE();
}


void cpuid_print_feature_info()
{

    u32 info[4]; // family info, addiitonal features, featureflag2, featureflag1
    __asm__("mov $0x1 , %eax\n\t");
    asm volatile("cpuid\n\t":"=a"(info[0]), "=b" (info[1]), "=c" (info[2]), "=d" (info[3]));

    for (size_t i = 0; i < 4; i++)
    {
        decToBinary(info[i]);
        LOG("Feature info ", i, ": ");
        for (size_t c = 0; c < 32; c++)
        {
            WRITE(decimal[binaryNum[c]]);
        }
        NEWLINE();
    }
}


void cpuid_get_core_crystal_freq()
{

    if (0x15 > max_param)
    {
        LOG("ERROR: Cannot get timing info. CPUID feature not supported.");
        return;
    }
    u32 info[6]; //denom, numer, freq,  base, max, reference;
    asm volatile ("mov $0x15 , %eax\n\t");
    asm volatile("cpuid\n\t":"=a"(info[0]), "=b" (info[1]), "=c" (info[2]));
    asm volatile("mov $0x16 , %eax\n\t");
    asm volatile("cpuid\n\t":"=a"(info[3]), "=b" (info[4]), "=c" (info[5]));
    for (size_t i = 0; i < 6; i++)
    {
        LOG("Timing info ", i, ": ");
        WRITE(info[i]);
        NEWLINE();
    }
}
