#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include "header/stdlib/string.h"
#include "header/memory/paging.h"

__attribute__((aligned(0x1000))) struct PageDirectory _paging_kernel_page_directory = {
    .table = {
        [0] = {
            .flag.present_bit = 1,
            .flag.write_bit = 1,
            .flag.use_pagesize_4_mb = 1,
            .lower_address = 0,
        },
        [0x300] = {
            .flag.present_bit = 1,
            .flag.write_bit = 1,
            .flag.use_pagesize_4_mb = 1,
            .lower_address = 0,
        },
        // Map 0xC0400000 (4MB-8MB) for kernel data/bss expansion
        [0x301] = {
            .flag.present_bit = 1,
            .flag.write_bit = 1,
            .flag.use_pagesize_4_mb = 1,
            .lower_address = 1,  // Physical 4MB
        },
        // Map 0xC0800000 (8MB-12MB) for kernel heap
        [0x302] = {
            .flag.present_bit = 1,
            .flag.write_bit = 1,
            .flag.use_pagesize_4_mb = 1,
            .lower_address = 2,  // Physical 8MB
        },
    }};

__attribute__((aligned(0x1000))) static struct PageDirectory page_directory_list[PAGING_DIRECTORY_TABLE_MAX_COUNT] = {0};

static struct PageManagerState page_manager_state = {
    // Reserve first three 4MB frames for kernel identity + higher-half (0, 4MB, 8MB)
    .page_frame_map = {
        [0 ... 2] = true,
        [3 ... PAGE_FRAME_MAX_COUNT - 1] = false},
    .free_page_frame_count = PAGE_FRAME_MAX_COUNT - 3,
    .last_allocated_frame_index = 2,
};

static struct
{
    bool page_directory_used[PAGING_DIRECTORY_TABLE_MAX_COUNT];
} page_directory_manager = {
    .page_directory_used = {false},
};

void update_page_directory_entry(
    struct PageDirectory *page_dir,
    void *physical_addr,
    void *virtual_addr,
    struct PageDirectoryEntryFlag flag)
{
    uint32_t page_index = ((uint32_t)virtual_addr >> 22) & 0x3FF;
    page_dir->table[page_index].flag = flag;
    page_dir->table[page_index].lower_address = ((uint32_t)physical_addr >> 22) & 0x3FF;
    flush_single_tlb(virtual_addr);
}

void flush_single_tlb(void *virtual_addr)
{
    asm volatile("invlpg (%0)" : /* <Empty> */ : "b"(virtual_addr) : "memory");
}

/* --- Memory Management --- */
bool paging_allocate_check(uint32_t amount)
{
    uint32_t required_frames = (amount + PAGE_FRAME_SIZE - 1) / PAGE_FRAME_SIZE;
    return required_frames <= page_manager_state.free_page_frame_count;
}

bool paging_allocate_user_page_frame(struct PageDirectory *page_dir, void *virtual_addr)
{
    if (((uint32_t)virtual_addr >= 0XC0000000))
        return false; // virtual_addr is kernel space

    if (page_manager_state.free_page_frame_count == 0)
        return false; // No free page frames available

    if (((uint32_t)virtual_addr & 0x003FFFFF) != 0)
        return false; // virtual_addr is 4MB not aligned

    // Find free page frame

    uint32_t start_frame = (page_manager_state.last_allocated_frame_index + 1) % PAGE_FRAME_MAX_COUNT;
    uint32_t frame_index = start_frame;
    do
    {
        if (!page_manager_state.page_frame_map[frame_index] && frame_index != 0)
        {
            // Found free frame
            page_manager_state.page_frame_map[frame_index] = true;
            page_manager_state.free_page_frame_count--;
            page_manager_state.last_allocated_frame_index = (frame_index + 1) % PAGE_FRAME_MAX_COUNT;

            void *physical_addr = (void *)(frame_index * PAGE_FRAME_SIZE);

            struct PageDirectoryEntryFlag flag = {
                .present_bit = 1,
                .write_bit = 1,
                .user_supervisor = 1,
                .page_write_through = 0,
                .page_cache_disable = 0,
                .accessed_bit = 0,
                .dirty_bit = 0,
                .use_pagesize_4_mb = 1,
            };

            update_page_directory_entry(page_dir, physical_addr, virtual_addr, flag);
            return true;
        }
        frame_index = (frame_index + 1) % PAGE_FRAME_MAX_COUNT;
    } while (frame_index != start_frame);

    return false;
}

bool paging_free_user_page_frame(struct PageDirectory *page_dir, void *virtual_addr)
{
    if (((uint32_t)virtual_addr >= 0xC0000000))
        return false; // virtual_addr is kernel space

    uint32_t page_index = ((uint32_t)virtual_addr >> 22) & 0x3FF;

    if (page_dir->table[page_index].flag.present_bit == 0)
        return false; // Page not present

    if (page_dir->table[page_index].flag.user_supervisor == 0)
        return false; // Not a user page

    uint32_t frame_index = page_dir->table[page_index].lower_address;

    if (frame_index == 0 || frame_index >= PAGE_FRAME_MAX_COUNT)
        return false; // Invalid frame index

    // Mark frame as free
    page_manager_state.page_frame_map[frame_index] = false;
    page_manager_state.free_page_frame_count++;

    // Clear page directory entry
    struct PageDirectoryEntryFlag flag = {0};
    update_page_directory_entry(page_dir, NULL, virtual_addr, flag);

    return true;
}

struct PageDirectory *paging_create_new_page_directory(void)
{
    for (size_t i = 0; i < PAGING_DIRECTORY_TABLE_MAX_COUNT; i++)
    {
        if (!page_directory_manager.page_directory_used[i])
        {
            // Found unused page directory
            page_directory_manager.page_directory_used[i] = true;

            memset(&page_directory_list[i], 0, sizeof(struct PageDirectory));

            struct PageDirectoryEntryFlag kernel_flag = {
                .present_bit = 1,
                .write_bit = 1,
                .use_pagesize_4_mb = 1,
                .user_supervisor = 0,
                .page_write_through = 0,
                .page_cache_disable = 0,
                .accessed_bit = 0,
                .dirty_bit = 0,
            };

            // Map identity page (first 4MB)
            page_directory_list[i].table[0].flag = kernel_flag;
            page_directory_list[i].table[0].lower_address = 0;

            // Map kernel at 0xC0000000 (first 4MB physical)
            page_directory_list[i].table[0x300].flag = kernel_flag;
            page_directory_list[i].table[0x300].lower_address = 0;

            // Map 0xC0400000 (4MB-8MB physical)
            page_directory_list[i].table[0x301].flag = kernel_flag;
            page_directory_list[i].table[0x301].lower_address = 1;

            // Map 0xC0800000 for kernel heap (8MB-12MB physical)
            page_directory_list[i].table[0x302].flag = kernel_flag;
            page_directory_list[i].table[0x302].lower_address = 2;

            return &page_directory_list[i];
        }
    }

    return NULL;
}

bool paging_free_page_directory(struct PageDirectory *page_dir)
{
    for (size_t i = 0; i < PAGING_DIRECTORY_TABLE_MAX_COUNT; i++)
    {
        if (&page_directory_list[i] == page_dir)
        {
            for (size_t j = 0; j < PAGE_ENTRY_COUNT; j++)
            {
                if (page_dir->table[j].flag.present_bit == 1 && page_dir->table[j].flag.user_supervisor == 1)
                {
                    uint32_t frame = page_directory_list[i].table[j].lower_address;
                    if (frame < PAGE_FRAME_MAX_COUNT && frame != 0)
                    {
                        page_manager_state.page_frame_map[frame] = false;
                        page_manager_state.free_page_frame_count++;
                    }
                }
            }
            page_directory_manager.page_directory_used[i] = false;
            memset(&page_directory_list[i], 0, sizeof(struct PageDirectory));
            return true;
        }
    }
    return false;
}

struct PageDirectory *paging_get_current_page_directory_addr(void)
{
    uint32_t current_page_directory_phys_addr;
    __asm__ volatile("mov %%cr3, %0" : "=r"(current_page_directory_phys_addr) : /* <Empty> */);
    uint32_t virtual_addr_page_dir = current_page_directory_phys_addr + KERNEL_VIRTUAL_ADDRESS_BASE;
    return (struct PageDirectory *)virtual_addr_page_dir;
}

void paging_use_page_directory(struct PageDirectory *page_dir_virtual_addr)
{
    uint32_t physical_addr_page_dir = (uint32_t)page_dir_virtual_addr;
    // Additional layer of check & mistake safety net
    if ((uint32_t)page_dir_virtual_addr > KERNEL_VIRTUAL_ADDRESS_BASE)
        physical_addr_page_dir -= KERNEL_VIRTUAL_ADDRESS_BASE;
    __asm__ volatile("mov %0, %%cr3" : /* <Empty> */ : "r"(physical_addr_page_dir) : "memory");
}