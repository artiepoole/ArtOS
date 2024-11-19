/*   Copyright (C) 1999,2003,2007,2008,2009,2010  Free Software Foundation, Inc.
 *
 *  Permission is hereby granted, free of charge, to any person obtaining a copy
 *  of this software and associated documentation files (the "Software"), to
 *  deal in the Software without restriction, including without limitation the
 *  rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
 *  sell copies of the Software, and to permit persons to whom the Software is
 *  furnished to do so, subject to the following conditions:
 *
 *  The above copyright notice and this permission notice shall be included in
 *  all copies or substantial portions of the Software.
 *
 *  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 *  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 *  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL ANY
 *  DEVELOPER OR DISTRIBUTOR BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 *  WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR
 *  IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

/* Modified by Artie Poole on 14/08/2024 */

#ifndef MULTIBOOT2_HEADER
#define MULTIBOOT2_HEADER 1

/*  How many bytes from the start of the file we search for the header. */
#define MULTIBOOT2_SEARCH                        32768
#define MULTIBOOT2_HEADER_ALIGN                  8

/*  The magic field should contain this. */
#define MULTIBOOT2_HEADER_MAGIC                 0xe85250d6

/*  This should be in %eax. */
#define MULTIBOOT2_BOOTLOADER_MAGIC             0x36d76289

/*  Alignment of multiboot2 modules. */
#define MULTIBOOT2_MOD_ALIGN                     0x00001000

/*  Alignment of the multiboot2 info structure. */
#define MULTIBOOT2_INFO_ALIGN                    0x00000008

/*  Flags set in the ’flags’ member of the multiboot2 header. */

#define MULTIBOOT2_TAG_ALIGN                  8
#define MULTIBOOT2_TAG_TYPE_END               0
#define MULTIBOOT2_TAG_TYPE_CMDLINE           1
#define MULTIBOOT2_TAG_TYPE_BOOT_LOADER_NAME  2
#define MULTIBOOT2_TAG_TYPE_MODULE            3
#define MULTIBOOT2_TAG_TYPE_BASIC_MEMINFO     4
#define MULTIBOOT2_TAG_TYPE_BOOTDEV           5
#define MULTIBOOT2_TAG_TYPE_MMAP              6
#define MULTIBOOT2_TAG_TYPE_VBE               7
#define MULTIBOOT2_TAG_TYPE_FRAMEBUFFER       8
#define MULTIBOOT2_TAG_TYPE_ELF_SECTIONS      9
#define MULTIBOOT2_TAG_TYPE_APM               10
#define MULTIBOOT2_TAG_TYPE_EFI32             11
#define MULTIBOOT2_TAG_TYPE_EFI64             12
#define MULTIBOOT2_TAG_TYPE_SMBIOS            13
#define MULTIBOOT2_TAG_TYPE_ACPI_OLD          14
#define MULTIBOOT2_TAG_TYPE_ACPI_NEW          15
#define MULTIBOOT2_TAG_TYPE_NETWORK           16
#define MULTIBOOT2_TAG_TYPE_EFI_MMAP          17
#define MULTIBOOT2_TAG_TYPE_EFI_BS            18
#define MULTIBOOT2_TAG_TYPE_EFI32_IH          19
#define MULTIBOOT2_TAG_TYPE_EFI64_IH          20
#define MULTIBOOT2_TAG_TYPE_LOAD_BASE_ADDR    21

#define MULTIBOOT2_HEADER_TAG_END  0
#define MULTIBOOT2_HEADER_TAG_INFORMATION_REQUEST  1
#define MULTIBOOT2_HEADER_TAG_ADDRESS  2
#define MULTIBOOT2_HEADER_TAG_ENTRY_ADDRESS  3
#define MULTIBOOT2_HEADER_TAG_CONSOLE_FLAGS  4
#define MULTIBOOT2_HEADER_TAG_FRAMEBUFFER  5
#define MULTIBOOT2_HEADER_TAG_MODULE_ALIGN  6
#define MULTIBOOT2_HEADER_TAG_EFI_BS        7
#define MULTIBOOT2_HEADER_TAG_ENTRY_ADDRESS_EFI32  8
#define MULTIBOOT2_HEADER_TAG_ENTRY_ADDRESS_EFI64  9
#define MULTIBOOT2_HEADER_TAG_RELOCATABLE  10

#define MULTIBOOT2_ARCHITECTURE_I386  0
#define MULTIBOOT2_ARCHITECTURE_MIPS32  4
#define MULTIBOOT2_HEADER_TAG_OPTIONAL 1

#define MULTIBOOT2_LOAD_PREFERENCE_NONE 0
#define MULTIBOOT2_LOAD_PREFERENCE_LOW 1
#define MULTIBOOT2_LOAD_PREFERENCE_HIGH 2

#define MULTIBOOT2_CONSOLE_FLAGS_CONSOLE_REQUIRED 1
#define MULTIBOOT2_CONSOLE_FLAGS_EGA_TEXT_SUPPORTED 2

#if FORLAPTOP
    #define TARGET_WIDTH 1920
    #define TARGET_HEIGHT 1080
#else
    #define TARGET_WIDTH 1280
    #define TARGET_HEIGHT 960
#endif


#ifndef ASM_FILE

typedef unsigned char multiboot2_uint8_t;
typedef unsigned short multiboot2_uint16_t;
typedef unsigned int multiboot2_uint32_t;
typedef unsigned long long multiboot2_uint64_t;

struct multiboot2_header
{
    /*  Must be MULTIBOOT2_MAGIC - see above. */
    multiboot2_uint32_t magic;

    /*  ISA */
    multiboot2_uint32_t architecture;

    /*  Total header length. */
    multiboot2_uint32_t header_length;

    /*  The above fields plus this one must equal 0 mod 2^32. */
    multiboot2_uint32_t checksum;
};

struct multiboot2_header_tag
{
    multiboot2_uint16_t type;
    multiboot2_uint16_t flags;
    multiboot2_uint32_t size;
};

struct multiboot2_header_tag_information_request
{
    multiboot2_uint16_t type;
    multiboot2_uint16_t flags;
    multiboot2_uint32_t size;
    multiboot2_uint32_t requests[0];
};

struct multiboot2_header_tag_address
{
    multiboot2_uint16_t type;
    multiboot2_uint16_t flags;
    multiboot2_uint32_t size;
    multiboot2_uint32_t header_addr;
    multiboot2_uint32_t load_addr;
    multiboot2_uint32_t load_end_addr;
    multiboot2_uint32_t bss_end_addr;
};

struct multiboot2_header_tag_entry_address
{
    multiboot2_uint16_t type;
    multiboot2_uint16_t flags;
    multiboot2_uint32_t size;
    multiboot2_uint32_t entry_addr;
};

struct multiboot2_header_tag_console_flags
{
    multiboot2_uint16_t type;
    multiboot2_uint16_t flags;
    multiboot2_uint32_t size;
    multiboot2_uint32_t console_flags;
};

struct multiboot2_header_tag_framebuffer
{
    multiboot2_uint16_t type;
    multiboot2_uint16_t flags;
    multiboot2_uint32_t size;
    multiboot2_uint32_t width;
    multiboot2_uint32_t height;
    multiboot2_uint32_t depth;
};

struct multiboot2_header_tag_module_align
{
    multiboot2_uint16_t type;
    multiboot2_uint16_t flags;
    multiboot2_uint32_t size;
};

struct multiboot2_header_tag_relocatable
{
    multiboot2_uint16_t type;
    multiboot2_uint16_t flags;
    multiboot2_uint32_t size;
    multiboot2_uint32_t min_addr;
    multiboot2_uint32_t max_addr;
    multiboot2_uint32_t align;
    multiboot2_uint32_t preference;
};

struct multiboot2_color
{
    multiboot2_uint8_t red;
    multiboot2_uint8_t green;
    multiboot2_uint8_t blue;
};

struct multiboot2_mmap_entry
{
    multiboot2_uint64_t addr;
    multiboot2_uint64_t len;
#define MULTIBOOT2_MEMORY_AVAILABLE              1
#define MULTIBOOT2_MEMORY_RESERVED               2
#define MULTIBOOT2_MEMORY_ACPI_RECLAIMABLE       3
#define MULTIBOOT2_MEMORY_NVS                    4
#define MULTIBOOT2_MEMORY_BADRAM                 5
    multiboot2_uint32_t type;
    multiboot2_uint32_t zero;
};

typedef struct multiboot2_mmap_entry multiboot2_memory_map_t;

struct multiboot2_tag
{
    multiboot2_uint32_t type;
    multiboot2_uint32_t size;
};

struct multiboot2_tag_string
{
    multiboot2_uint32_t type;
    multiboot2_uint32_t size;
    char string[0];
};

struct multiboot2_tag_module
{
    multiboot2_uint32_t type;
    multiboot2_uint32_t size;
    multiboot2_uint32_t mod_start;
    multiboot2_uint32_t mod_end;
    char cmdline[0];
};

struct multiboot2_tag_basic_meminfo
{
    multiboot2_uint32_t type;
    multiboot2_uint32_t size;
    multiboot2_uint32_t mem_lower;
    multiboot2_uint32_t mem_upper;
};

struct multiboot2_tag_bootdev
{
    multiboot2_uint32_t type;
    multiboot2_uint32_t size;
    multiboot2_uint32_t biosdev;
    multiboot2_uint32_t partition;
    multiboot2_uint32_t sub_partition;
};

struct multiboot2_tag_mmap
{
    multiboot2_uint32_t type;
    multiboot2_uint32_t size;
    multiboot2_uint32_t entry_size;
    multiboot2_uint32_t entry_version;
    multiboot2_mmap_entry* entries[6];
};

struct multiboot2_vbe_info_block
{
    multiboot2_uint8_t external_specification[512];
};

struct multiboot2_vbe_mode_info_block
{
    multiboot2_uint8_t external_specification[256];
};

struct multiboot2_tag_vbe
{
    multiboot2_uint32_t type;
    multiboot2_uint32_t size;

    multiboot2_uint16_t vbe_mode;
    multiboot2_uint16_t vbe_interface_seg;
    multiboot2_uint16_t vbe_interface_off;
    multiboot2_uint16_t vbe_interface_len;

    multiboot2_vbe_info_block vbe_control_info;
    multiboot2_vbe_mode_info_block vbe_mode_info;
};

struct multiboot2_tag_framebuffer_common
{
    multiboot2_uint32_t type;
    multiboot2_uint32_t size;

    multiboot2_uint64_t framebuffer_addr;
    multiboot2_uint32_t framebuffer_pitch;
    multiboot2_uint32_t framebuffer_width;
    multiboot2_uint32_t framebuffer_height;
    multiboot2_uint8_t framebuffer_bpp;
#define MULTIBOOT2_FRAMEBUFFER_TYPE_INDEXED 0
#define MULTIBOOT2_FRAMEBUFFER_TYPE_RGB     1
#define MULTIBOOT2_FRAMEBUFFER_TYPE_EGA_TEXT     2
    multiboot2_uint8_t framebuffer_type;
    multiboot2_uint16_t reserved;
};

struct multiboot2_tag_framebuffer
{
    multiboot2_tag_framebuffer_common common;

    union
    {
        struct
        {
            multiboot2_uint16_t framebuffer_palette_num_colors;
            multiboot2_color* framebuffer_palette;
        };

        struct
        {
            multiboot2_uint8_t framebuffer_red_field_position;
            multiboot2_uint8_t framebuffer_red_mask_size;
            multiboot2_uint8_t framebuffer_green_field_position;
            multiboot2_uint8_t framebuffer_green_mask_size;
            multiboot2_uint8_t framebuffer_blue_field_position;
            multiboot2_uint8_t framebuffer_blue_mask_size;
        };
    };
};

struct multiboot2_tag_elf_sections
{
    multiboot2_uint32_t type;
    multiboot2_uint32_t size;
    multiboot2_uint32_t num;
    multiboot2_uint32_t entsize;
    multiboot2_uint32_t shndx;
    char* sections;
};

struct multiboot2_tag_apm
{
    multiboot2_uint32_t type;
    multiboot2_uint32_t size;
    multiboot2_uint16_t version;
    multiboot2_uint16_t cseg;
    multiboot2_uint32_t offset;
    multiboot2_uint16_t cseg_16;
    multiboot2_uint16_t dseg;
    multiboot2_uint16_t flags;
    multiboot2_uint16_t cseg_len;
    multiboot2_uint16_t cseg_16_len;
    multiboot2_uint16_t dseg_len;
};

struct multiboot2_tag_efi32
{
    multiboot2_uint32_t type;
    multiboot2_uint32_t size;
    multiboot2_uint32_t pointer;
};

struct multiboot2_tag_efi64
{
    multiboot2_uint32_t type;
    multiboot2_uint32_t size;
    multiboot2_uint64_t pointer;
};

struct multiboot2_tag_smbios
{
    multiboot2_uint32_t type;
    multiboot2_uint32_t size;
    multiboot2_uint8_t major;
    multiboot2_uint8_t minor;
    multiboot2_uint8_t reserved[6];
    multiboot2_uint32_t* tables;
};

struct RSDP_v1
{
    char Signature[8];
    multiboot2_uint8_t Checksum;
    char OEMID[6];
    multiboot2_uint8_t Revision;
    multiboot2_uint32_t* RsdtAddress;
} __attribute__ ((packed));

struct RSDP_v2
{
    char Signature[8];
    multiboot2_uint8_t Checksum;
    char OEMID[6];
    multiboot2_uint8_t Revision;
    multiboot2_uint32_t* RsdtAddress;
    multiboot2_uint32_t length;
    multiboot2_uint64_t XsdtAddress;
    multiboot2_uint8_t ExChecksum;
    multiboot2_uint8_t reserved[3];
} __attribute__ ((packed));

struct multiboot2_tag_old_acpi
{
    multiboot2_uint32_t type;
    multiboot2_uint32_t size;
    RSDP_v1 rsdp;
} __attribute__ ((packed));;

struct multiboot2_tag_new_acpi
{
    multiboot2_uint32_t type;
    multiboot2_uint32_t size;
    RSDP_v2 rsdp;
} __attribute__ ((packed));;

struct multiboot2_tag_network
{
    multiboot2_uint32_t type;
    multiboot2_uint32_t size;
    multiboot2_uint32_t* dhcpack;
} __attribute__ ((packed));;

struct multiboot2_tag_efi_mmap
{
    multiboot2_uint32_t type;
    multiboot2_uint32_t size;
    multiboot2_uint32_t descr_size;
    multiboot2_uint32_t descr_vers;
    multiboot2_uint32_t* efi_mmap;
} __attribute__ ((packed));;

struct multiboot2_tag_efi32_ih
{
    multiboot2_uint32_t type;
    multiboot2_uint32_t size;
    multiboot2_uint32_t* pointer;
} __attribute__ ((packed));;

struct multiboot2_tag_efi64_ih
{
    multiboot2_uint32_t type;
    multiboot2_uint32_t size;
    multiboot2_uint64_t pointer;
} __attribute__ ((packed));;

struct multiboot2_tag_load_base_addr
{
    multiboot2_uint32_t type;
    multiboot2_uint32_t size;
    multiboot2_uint32_t load_base_addr;
} __attribute__ ((packed));;

struct multiboot2_tag_info_cmd_line
{
    multiboot2_uint32_t type;
    multiboot2_uint32_t size;
    multiboot2_uint8_t load_base_addr;

} __attribute__ ((packed));;


struct artos_boot_header
{
    multiboot2_tag_load_base_addr base_addr;
    multiboot2_tag_info_cmd_line cmd_info;
    multiboot2_tag_framebuffer_common framebuffer_common ;
    multiboot2_tag_elf_sections elf_sections;
    multiboot2_tag_new_acpi new_acpi;
    multiboot2_tag_old_acpi old_acpi;
    multiboot2_tag_smbios smbios;
    multiboot2_tag_bootdev bootdev;
    multiboot2_tag_basic_meminfo meminfo;
    multiboot2_tag_mmap mmap;
    multiboot2_tag_apm apm;
    multiboot2_tag_vbe vbe;
};

artos_boot_header* multiboot2_populate(multiboot2_uint32_t boot_info_address);
multiboot2_uint32_t multiboot2_get_APIC_address();
multiboot2_uint32_t multiboot2_get_MADT_table_address();


#endif /*  ! ASM_FILE */

#endif /*  ! MULTIBOOT2_HEADER */
