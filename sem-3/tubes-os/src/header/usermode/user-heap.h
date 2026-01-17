#ifndef _USER_HEAP_H
#define _USER_HEAP_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

/**
 * User-space Heap Memory Functions
 * 
 * These functions provide user-space programs access to kernel heap
 * through syscalls.
 */

/**
 * Heap Statistics structure (same as kernel)
 */
typedef struct {
    uint32_t total_size;
    uint32_t used_size;
    uint32_t free_size;
    uint32_t block_count;
    uint32_t free_block_count;
    uint32_t alloc_count;
    uint32_t free_count;
} UserHeapStats;

/**
 * Heap Test Result structure
 */
typedef struct {
    bool test1_alloc;      // Small allocation success
    bool test2_alloc;      // Another allocation success  
    bool test3_calloc;     // Calloc success
    bool test3_zeroed;     // Calloc was zeroed
    bool test4_free;       // Free success
    bool test5_realloc;    // Realloc/reuse success
    bool test6_cleanup;    // Cleanup success
    uint32_t alloc_count;  // Total allocations
    uint32_t free_count;   // Total frees
} HeapTestResult;

/**
 * Run heap test in kernel mode
 * 
 * @param result Pointer to HeapTestResult structure to fill
 */
void heap_test(HeapTestResult *result);

/**
 * Allocate memory
 * 
 * @param size  Number of bytes to allocate
 * @return      Pointer to allocated memory, or NULL if allocation fails
 */
void *malloc(uint32_t size);

/**
 * Allocate zeroed memory
 * 
 * @param nmemb Number of elements
 * @param size  Size of each element
 * @return      Pointer to zeroed allocated memory, or NULL if allocation fails
 */
void *calloc(uint32_t nmemb, uint32_t size);

/**
 * Reallocate memory block
 * 
 * @param ptr   Pointer to previously allocated memory (or NULL)
 * @param size  New size in bytes
 * @return      Pointer to reallocated memory, or NULL if reallocation fails
 */
void *realloc(void *ptr, uint32_t size);

/**
 * Free previously allocated memory
 * 
 * @param ptr   Pointer to memory to free (NULL is safely ignored)
 */
void free(void *ptr);

/**
 * Get heap statistics (for debugging)
 * 
 * @param stats Pointer to UserHeapStats structure to fill
 */
void heap_stats(UserHeapStats *stats);

#endif
