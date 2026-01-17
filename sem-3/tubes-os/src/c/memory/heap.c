#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include "header/memory/heap.h"
#include "header/stdlib/string.h"

/**
 * Kernel Heap Memory Allocator Implementation
 * 
 * This is a first-fit free list allocator with coalescing.
 * 
 * Memory Layout:
 * [HeapBlock Header][User Data][HeapBlock Header][User Data]...
 * 
 * The heap is a doubly-linked list of blocks. Adjacent free blocks
 * are merged (coalesced) when memory is freed.
 */

// Global heap state
static HeapBlock *heap_start = NULL;
static bool heap_initialized = false;

// Statistics
static HeapStats heap_stats = {0};

/**
 * Align size up to specified alignment
 */
static inline uint32_t align_up(uint32_t value, uint32_t alignment) {
    return (value + alignment - 1) & ~(alignment - 1);
}

/**
 * Get pointer to user data from block header
 */
static inline void *block_to_ptr(HeapBlock *block) {
    return (void *)((uint8_t *)block + sizeof(HeapBlock));
}

/**
 * Get block header from user data pointer
 */
static inline HeapBlock *ptr_to_block(void *ptr) {
    return (HeapBlock *)((uint8_t *)ptr - sizeof(HeapBlock));
}

/**
 * Check if a block is valid
 */
static inline bool is_valid_block(HeapBlock *block) {
    if (block == NULL) return false;
    if ((uint32_t)block < KERNEL_HEAP_START) return false;
    if ((uint32_t)block >= KERNEL_HEAP_END) return false;
    if (block->magic != HEAP_BLOCK_MAGIC) return false;
    return true;
}

/**
 * Split a block if it's large enough
 * Creates a new free block from the remaining space
 */
static void split_block(HeapBlock *block, uint32_t size) {
    uint32_t min_split_size = sizeof(HeapBlock) + HEAP_MIN_ALLOC_SIZE;
    
    // Only split if remaining space is large enough for a new block
    if (block->size >= size + min_split_size) {
        // Create new block after the allocated space
        HeapBlock *new_block = (HeapBlock *)((uint8_t *)block + sizeof(HeapBlock) + size);
        
        new_block->magic = HEAP_BLOCK_MAGIC;
        new_block->size = block->size - size - sizeof(HeapBlock);
        new_block->is_free = true;
        new_block->next = block->next;
        new_block->prev = block;
        
        // Update linked list
        if (block->next != NULL) {
            block->next->prev = new_block;
        }
        block->next = new_block;
        
        // Update original block size
        block->size = size;
        
        // Update statistics
        heap_stats.block_count++;
        heap_stats.free_block_count++;
    }
}

/**
 * Merge a block with its next neighbor if both are free
 */
static void coalesce_next(HeapBlock *block) {
    if (block == NULL || block->next == NULL) return;
    if (!block->is_free || !block->next->is_free) return;
    
    HeapBlock *next = block->next;
    
    // Absorb next block
    block->size += sizeof(HeapBlock) + next->size;
    block->next = next->next;
    
    if (next->next != NULL) {
        next->next->prev = block;
    }
    
    // Invalidate merged block
    next->magic = 0;
    
    // Update statistics
    heap_stats.block_count--;
    heap_stats.free_block_count--;
}

/**
 * Merge a block with its previous neighbor if both are free
 */
static void coalesce_prev(HeapBlock *block) {
    if (block == NULL || block->prev == NULL) return;
    if (!block->is_free || !block->prev->is_free) return;
    
    // Let previous block absorb this one
    coalesce_next(block->prev);
}

void heap_init(void) {
    if (heap_initialized) return;
    
    // Initialize the heap with one large free block
    heap_start = (HeapBlock *)KERNEL_HEAP_START;
    
    heap_start->magic = HEAP_BLOCK_MAGIC;
    heap_start->size = KERNEL_HEAP_SIZE - sizeof(HeapBlock);
    heap_start->is_free = true;
    heap_start->next = NULL;
    heap_start->prev = NULL;
    
    // Initialize statistics
    heap_stats.total_size = KERNEL_HEAP_SIZE;
    heap_stats.used_size = 0;
    heap_stats.free_size = heap_start->size;
    heap_stats.block_count = 1;
    heap_stats.free_block_count = 1;
    heap_stats.alloc_count = 0;
    heap_stats.free_count = 0;
    
    heap_initialized = true;
}

void *kmalloc(uint32_t size) {
    if (!heap_initialized) {
        heap_init();
    }
    
    if (size == 0) return NULL;
    
    // Align size to 4 bytes for better memory access
    size = align_up(size, 4);
    
    // Ensure minimum allocation size
    if (size < HEAP_MIN_ALLOC_SIZE) {
        size = HEAP_MIN_ALLOC_SIZE;
    }
    
    // First-fit search
    HeapBlock *current = heap_start;
    
    while (current != NULL) {
        if (current->is_free && current->size >= size) {
            // Found a suitable block
            
            // Split if block is too large
            split_block(current, size);
            
            // Mark as allocated
            current->is_free = false;
            
            // Update statistics
            heap_stats.used_size += current->size + sizeof(HeapBlock);
            heap_stats.free_size -= current->size + sizeof(HeapBlock);
            heap_stats.free_block_count--;
            heap_stats.alloc_count++;
            
            return block_to_ptr(current);
        }
        current = current->next;
    }
    
    // No suitable block found
    return NULL;
}

void *kzalloc(uint32_t size) {
    void *ptr = kmalloc(size);
    if (ptr != NULL) {
        memset(ptr, 0, size);
    }
    return ptr;
}

void *kmalloc_aligned(uint32_t size, uint32_t alignment) {
    if (!heap_initialized) {
        heap_init();
    }
    
    if (size == 0 || alignment == 0) return NULL;
    
    // Alignment must be power of 2
    if ((alignment & (alignment - 1)) != 0) return NULL;
    
    // Allocate extra space to ensure we can align
    uint32_t total_size = size + alignment + sizeof(void *);
    
    void *raw_ptr = kmalloc(total_size);
    if (raw_ptr == NULL) return NULL;
    
    // Calculate aligned address
    uint32_t raw_addr = (uint32_t)raw_ptr;
    uint32_t aligned_addr = align_up(raw_addr + sizeof(void *), alignment);
    
    // Store original pointer just before the aligned address
    *((void **)(aligned_addr - sizeof(void *))) = raw_ptr;
    
    return (void *)aligned_addr;
}

void *krealloc(void *ptr, uint32_t size) {
    if (ptr == NULL) {
        return kmalloc(size);
    }
    
    if (size == 0) {
        kfree(ptr);
        return NULL;
    }
    
    HeapBlock *block = ptr_to_block(ptr);
    
    if (!is_valid_block(block)) {
        return NULL;
    }
    
    // If current block is large enough, return same pointer
    if (block->size >= size) {
        return ptr;
    }
    
    // Allocate new block
    void *new_ptr = kmalloc(size);
    if (new_ptr == NULL) {
        return NULL;
    }
    
    // Copy old data
    memcpy(new_ptr, ptr, block->size);
    
    // Free old block
    kfree(ptr);
    
    return new_ptr;
}

void kfree(void *ptr) {
    if (ptr == NULL) return;
    
    if (!heap_initialized) return;
    
    HeapBlock *block = ptr_to_block(ptr);
    
    // Validate block
    if (!is_valid_block(block)) {
        // Invalid pointer or double free - silently ignore
        return;
    }
    
    if (block->is_free) {
        // Double free detected - silently ignore
        return;
    }
    
    // Mark as free
    block->is_free = true;
    
    // Update statistics
    heap_stats.used_size -= block->size + sizeof(HeapBlock);
    heap_stats.free_size += block->size + sizeof(HeapBlock);
    heap_stats.free_block_count++;
    heap_stats.free_count++;
    
    // Coalesce with adjacent free blocks
    coalesce_next(block);
    coalesce_prev(block);
}

void heap_get_stats(HeapStats *stats) {
    if (stats == NULL) return;
    
    if (!heap_initialized) {
        memset(stats, 0, sizeof(HeapStats));
        return;
    }
    
    memcpy(stats, &heap_stats, sizeof(HeapStats));
}

bool heap_is_valid_ptr(void *ptr) {
    if (ptr == NULL) return false;
    
    if (!heap_initialized) return false;
    
    // Check if pointer is within heap bounds
    uint32_t addr = (uint32_t)ptr;
    if (addr < KERNEL_HEAP_START + sizeof(HeapBlock)) return false;
    if (addr >= KERNEL_HEAP_END) return false;
    
    HeapBlock *block = ptr_to_block(ptr);
    
    return is_valid_block(block) && !block->is_free;
}

uint32_t heap_get_block_size(void *ptr) {
    if (ptr == NULL) return 0;
    
    if (!heap_initialized) return 0;
    
    HeapBlock *block = ptr_to_block(ptr);
    
    if (!is_valid_block(block)) return 0;
    
    return block->size;
}
