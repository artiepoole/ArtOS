//
// Created by artypoole on 16/08/24.
//


#include "multiboot2.h"
#include "logging.h"
#include "ACPI.h"
#include "stdlib.h"
#include "string.h"


artos_boot_header boot_info{};


/*
 * Populates the boot info structure. Takes the u32 direct from the stack on boot.
 */
artos_boot_header* multiboot2_populate(const multiboot2_uint32_t boot_info_address)
{
    const auto start_address = reinterpret_cast<u8*>(boot_info_address);
    auto target_addr = start_address + 8;
    const u32 size = *reinterpret_cast<u32*>(start_address); // first 4 bytes of this header is the size of the table

    // loop through each entry in the table
    while (target_addr < (start_address + size))
    {
        const auto tag = reinterpret_cast<multiboot2_tag*>(target_addr);
        switch (tag->type)
        {
        case MULTIBOOT2_TAG_TYPE_BOOT_LOADER_NAME:
            {
                [[maybe_unused]] char buffer[tag->size - 4];
                for (size_t i = 8; i < tag->size; i++)
                {
                    buffer[i - 8] = *reinterpret_cast<char*>(target_addr + i);
                }
                TIMESTAMP();
                WRITE("Boot info: bootloader name: ");
                WRITE(buffer);
                NEWLINE();
                break;
            }
        case MULTIBOOT2_TAG_TYPE_APM:
            LOG("Boot info: APM info loaded");
            boot_info.apm = *reinterpret_cast<multiboot2_tag_apm*>(target_addr);
            break;
        case MULTIBOOT2_TAG_TYPE_VBE:
            LOG("Boot info: VBE info loaded");
            boot_info.vbe = *reinterpret_cast<multiboot2_tag_vbe*>(target_addr);
            break;
        case MULTIBOOT2_TAG_TYPE_LOAD_BASE_ADDR:
            LOG("Boot info: base address loaded");
            boot_info.base_addr = *reinterpret_cast<multiboot2_tag_load_base_addr*>(target_addr);
            break;
        case MULTIBOOT2_TAG_TYPE_CMDLINE:
            LOG("Boot info: cmdline loaded");
            boot_info.cmd_info = *reinterpret_cast<multiboot2_tag_info_cmd_line*>(target_addr);
            break;
        case MULTIBOOT2_TAG_TYPE_FRAMEBUFFER:
            LOG("Boot info: framebuffer info loaded");
            boot_info.framebuffer_common = *reinterpret_cast<multiboot2_tag_framebuffer_common*>(target_addr);
        // TODO: populate this. to handle different colour types
            switch (boot_info.framebuffer_common.framebuffer_type)
            {
            case MULTIBOOT2_FRAMEBUFFER_TYPE_INDEXED:
                LOG("Framebuffer type: indexed");
                break;
            case MULTIBOOT2_FRAMEBUFFER_TYPE_RGB:
                LOG("Framebuffer type: rgb");
                break;
            case MULTIBOOT2_FRAMEBUFFER_TYPE_EGA_TEXT:
                LOG("Framebuffer type: ega text");
                break;
            default:
                LOG("Framebuffer type unknown. Type: ", boot_info.framebuffer_common.framebuffer_type);
            }
            break;
        case MULTIBOOT2_TAG_TYPE_ELF_SECTIONS:
            LOG("Boot info: ELF info loaded");
            boot_info.elf_sections = *reinterpret_cast<multiboot2_tag_elf_sections*>(target_addr);
            break;
        case MULTIBOOT2_TAG_TYPE_ACPI_NEW:
            LOG("Boot info: new ACPI info loaded");
            boot_info.new_acpi = *reinterpret_cast<multiboot2_tag_new_acpi*>(target_addr);
            break;
        case MULTIBOOT2_TAG_TYPE_ACPI_OLD:
            LOG("Boot info: new ACPI info loaded");
            boot_info.old_acpi = *reinterpret_cast<multiboot2_tag_old_acpi*>(target_addr);
        // todo: This needs to store the ACPI tables which are contained.
            break;
        case MULTIBOOT2_TAG_TYPE_SMBIOS:
            LOG("Boot info: SMBIOS info loaded");
            boot_info.smbios = *reinterpret_cast<multiboot2_tag_smbios*>(target_addr);
        // todo: need to actually get the smbios data. Getting grub errors. This needs to store the SMBIOS tables which are contained.
            break;
        case MULTIBOOT2_TAG_TYPE_BOOTDEV:
            LOG("Boot info: boot device info loaded");
            boot_info.bootdev = *reinterpret_cast<multiboot2_tag_bootdev*>(target_addr);
            break;
        case MULTIBOOT2_TAG_TYPE_BASIC_MEMINFO:
            LOG("Boot info: memory info loaded");
            boot_info.meminfo = *reinterpret_cast<multiboot2_tag_basic_meminfo*>(target_addr);
            break;
        case MULTIBOOT2_TAG_TYPE_MMAP:
            LOG("Boot info: memory map info loaded");
            boot_info.mmap = *reinterpret_cast<multiboot2_tag_mmap*>(target_addr);
            for (size_t i = 0; i < boot_info.mmap.size / boot_info.mmap.entry_size; i++)
            {
                boot_info.mmap.entries[i] = reinterpret_cast<multiboot2_mmap_entry*>(target_addr + 16 + (i * sizeof(multiboot2_mmap_entry)));
            }

            break;
        case MULTIBOOT2_TAG_TYPE_END:
            LOG("Boot info: done loading boot info.");
            break;
        default:
            LOG("Boot info: unhandled tag with id: ", tag->type);
            break;
        }
        // Next tag is always 8 bytes aligned (64-bit boundary)
        if (tag->size % 8 == 0)
            target_addr += tag->size;
        else
        {
            target_addr += tag->size + (8 - (tag->size % 8));
        }
    }
    return &boot_info;
}

/*
 *  Must call multiboot2_populate before use
 */
multiboot2_uint32_t multiboot2_get_APIC_address()
{
    return reinterpret_cast<RSDT*>(boot_info.old_acpi.rsdp.RsdtAddress)->madt_stub->local_apic_address;
}

multiboot2_uint32_t multiboot2_get_MADT_table_address()
{
    if (boot_info.old_acpi.type == 14) return reinterpret_cast<uintptr_t>(reinterpret_cast<RSDT*>(boot_info.old_acpi.rsdp.RsdtAddress)->madt_stub);
    if (boot_info.new_acpi.type == 15) return reinterpret_cast<uintptr_t>(reinterpret_cast<RSDT*>(boot_info.new_acpi.rsdp.RsdtAddress)->madt_stub);
    return 0;
}

/*
 *  Must call multiboot2_populate before use
 */
multiboot2_tag_framebuffer_common* multiboot2_get_framebuffer()
{
    return &boot_info.framebuffer_common;
}
