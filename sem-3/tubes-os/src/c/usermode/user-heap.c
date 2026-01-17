#include "header/usermode/user-heap.h"
#include "header/usermode/commands/syscall.h"
#include "header/stdlib/string.h"

/**
 * User-space Heap Memory Functions Implementation
 * 
 * These functions wrap syscalls to access kernel heap.
 */

// Use external syscall declaration from syscall.h
extern void syscall(uint32_t eax, uint32_t ebx, uint32_t ecx, uint32_t edx);

void *malloc(uint32_t size)
{
    if (size == 0) return NULL;
    
    void *result = NULL;
    syscall(SYS_MALLOC, size, (uint32_t)&result, 0);
    return result;
}

void *calloc(uint32_t nmemb, uint32_t size)
{
    if (nmemb == 0 || size == 0) return NULL;
    
    uint32_t total = nmemb * size;
    
    // Check for overflow
    if (total / nmemb != size) return NULL;
    
    void *ptr = malloc(total);
    if (ptr != NULL) {
        memset(ptr, 0, total);
    }
    return ptr;
}

void *realloc(void *ptr, uint32_t size)
{
    if (ptr == NULL) {
        return malloc(size);
    }
    
    if (size == 0) {
        free(ptr);
        return NULL;
    }
    
    void *result = NULL;
    syscall(SYS_REALLOC, (uint32_t)ptr, size, (uint32_t)&result);
    return result;
}

void free(void *ptr)
{
    if (ptr == NULL) return;
    syscall(SYS_FREE, (uint32_t)ptr, 0, 0);
}

void heap_stats(UserHeapStats *stats)
{
    if (stats == NULL) return;
    syscall(SYS_HEAP_STATS, (uint32_t)stats, 0, 0);
}

void heap_test(HeapTestResult *result)
{
    if (result == NULL) return;
    syscall(SYS_HEAP_TEST, (uint32_t)result, 0, 0);
}
