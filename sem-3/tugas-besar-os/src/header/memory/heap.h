#ifndef _HEAP_H
#define _HEAP_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

/**
 * Kernel Heap Memory Allocator
 * 
 * This implements a simple first-fit free list allocator for kernel heap.
 * The heap is located in kernel virtual address space starting at KERNEL_HEAP_START.
 */

// Kernel heap configuration
#define KERNEL_HEAP_START       0xC0800000  // Start address of kernel heap (8MB into kernel space)
#define KERNEL_HEAP_SIZE        0x100000    // 1MB kernel heap size
#define KERNEL_HEAP_END         (KERNEL_HEAP_START + KERNEL_HEAP_SIZE)

// Minimum allocation size (must be at least sizeof(HeapBlock))
#define HEAP_MIN_ALLOC_SIZE     16

// Magic number for heap block validation
#define HEAP_BLOCK_MAGIC        0xDEADBEEF

/**
 * Heap Block Header
 * 
 * Each allocated or free block starts with this header.
 * The actual user data follows immediately after the header.
 * 
 * @param magic     Magic number for corruption detection
 * @param size      Size of the data area (excluding header)
 * @param is_free   Whether this block is free
 * @param next      Pointer to next block in heap
 * @param prev      Pointer to previous block in heap
 */
typedef struct HeapBlock {
    uint32_t magic;
    uint32_t size;
    bool is_free;
    struct HeapBlock *next;
    struct HeapBlock *prev;
} __attribute__((packed)) HeapBlock;

/**
 * Heap Statistics for debugging
 */
typedef struct HeapStats {
    uint32_t total_size;        // Total heap size
    uint32_t used_size;         // Currently allocated size
    uint32_t free_size;         // Currently free size
    uint32_t block_count;       // Total number of blocks
    uint32_t free_block_count;  // Number of free blocks
    uint32_t alloc_count;       // Total allocations made
    uint32_t free_count;        // Total frees made
} HeapStats;

/**
 * Initialize the kernel heap
 * Must be called once during kernel initialization
 */
void heap_init(void);

/**
 * Allocate memory from kernel heap
 * 
 * @param size  Number of bytes to allocate
 * @return      Pointer to allocated memory, or NULL if allocation fails
 */
void *kmalloc(uint32_t size);

/**
 * Allocate zeroed memory from kernel heap
 * 
 * @param size  Number of bytes to allocate
 * @return      Pointer to zeroed allocated memory, or NULL if allocation fails
 */
void *kzalloc(uint32_t size);

/**
 * Allocate aligned memory from kernel heap
 * 
 * @param size      Number of bytes to allocate
 * @param alignment Required alignment (must be power of 2)
 * @return          Pointer to aligned allocated memory, or NULL if allocation fails
 */
void *kmalloc_aligned(uint32_t size, uint32_t alignment);

/**
 * Reallocate memory block
 * 
 * @param ptr   Pointer to previously allocated memory (or NULL)
 * @param size  New size in bytes
 * @return      Pointer to reallocated memory, or NULL if reallocation fails
 */
void *krealloc(void *ptr, uint32_t size);

/**
 * Free previously allocated memory
 * 
 * @param ptr   Pointer to memory to free (NULL is safely ignored)
 */
void kfree(void *ptr);

/**
 * Get heap statistics
 * 
 * @param stats Pointer to HeapStats structure to fill
 */
void heap_get_stats(HeapStats *stats);

/**
 * Check if a pointer was allocated by kmalloc
 * 
 * @param ptr   Pointer to check
 * @return      True if pointer is a valid heap allocation
 */
bool heap_is_valid_ptr(void *ptr);

/**
 * Get the size of an allocated block
 * 
 * @param ptr   Pointer to allocated memory
 * @return      Size of the allocation, or 0 if invalid
 */
uint32_t heap_get_block_size(void *ptr);

#endif
