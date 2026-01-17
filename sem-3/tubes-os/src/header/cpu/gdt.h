#ifndef _GDT_H
#define _GDT_H

#include <stdint.h>
#include "header/interrupt/interrupt.h"

// Some GDT Constant
#define GDT_MAX_ENTRY_COUNT 32
/**
 * As kernel SegmentDescriptor for code located at index 1 in GDT,
 * segment selector is sizeof(SegmentDescriptor) * 1 = 0x8
 */
#define GDT_KERNEL_CODE_SEGMENT_SELECTOR 0x8
#define GDT_KERNEL_DATA_SEGMENT_SELECTOR 0x10
#define GDT_USER_CODE_SEGMENT_SELECTOR 0x18
#define GDT_USER_DATA_SEGMENT_SELECTOR 0x20
#define GDT_TSS_SELECTOR               0x28

extern struct GDTR _gdt_gdtr;

/**
 * Segment Descriptor storing system segment information.
 * Struct defined exactly as Intel Manual Segment Descriptor definition (Figure 3-8 Segment Descriptor).
 * Manual can be downloaded at www.intel.com/content/www/us/en/architecture-and-technology/64-ia-32-architectures-software-developer-vol-3a-part-1-manual.html/
 *
 * @param segment_low  16-bit lower-bit segment limit
 * @param base_low     16-bit lower-bit base address
 * @param base_mid     8-bit middle-bit base address
 * @param type_bit     4-bit contain type flags
 * @param non_system   1-bit contain system
 * @param privilege    2-bit contain access level
 * @param presence     1-bit contain segment memory presence
 * @param segment_high 4-bit higher-bit segment limit
 * @param available    1-bit available for system use
 * @param long_mode    1-bit contain native 64-bit code
 * @param def_opr_size 1-bit contain default operation size
 * @param granularity  1-bit contain granularity
 * @param base_high    8-bit higher-bit base address
 */
struct SegmentDescriptor
{
    // 0 - 32
    uint16_t segment_low;     // 0:16
    uint16_t base_low;        // 16-31

    // 32 - 48
    uint8_t base_mid;         // 32:40
    uint8_t type_bit     : 4; // TYPE - Segment type
    uint8_t non_system   : 1; // S    - Descriptor type
    uint8_t privilege    : 2; // DPL  - Descriptor privilege level
    uint8_t presence     : 1; // P    - Segment present

    // 48 - 63
    uint8_t segment_high : 4; // 48:51
    uint8_t available    : 1; // AVL  - Available For Use
    uint8_t long_mode    : 1; // L    - 64-bit code segment
    uint8_t def_opr_size : 1; // D/B  - Default operation size (0 = 16-bit segment; 1 = 32-bit segment)
    uint8_t granularity  : 1; // G    - Granularity
    uint8_t base_high;        // 55:63

} __attribute__((packed));

/**
 * Global Descriptor Table containing list of segment descriptor. One GDT already defined in memory.c.
 * More details at https://wiki.osdev.org/GDT_Tutorial
 * @param table Fixed-width array of SegmentDescriptor with size GDT_MAX_ENTRY_COUNT
 */
struct GlobalDescriptorTable
{
    struct SegmentDescriptor table[GDT_MAX_ENTRY_COUNT];
} __attribute__((packed));

/**
 * GDTR, carrying information where's the GDT located and GDT size.
 * Global kernel variable defined at memory.c.
 *
 * @param size    Global Descriptor Table size, use sizeof operator
 * @param address GDT address, GDT should already defined properly
 */
struct GDTR
{
    uint16_t size;
    struct GlobalDescriptorTable *address;
} __attribute__((packed));


// Set GDT_TSS_SELECTOR with proper TSS values, accessing _interrupt_tss_entry
void gdt_install_tss(void);

#endif
