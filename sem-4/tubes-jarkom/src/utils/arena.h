/**
 * @file arena.h
 * @brief Bump-pointer arena allocator for hot-path packet buffers.
 */

#ifndef MAGI_UTILS_ARENA_H
#define MAGI_UTILS_ARENA_H

#include <stddef.h>
#include <stdint.h>

/**
 * @brief Default arena capacity (64 KB).
 */
#define ARENA_DEFAULT_CAPACITY (64U * 1024U)

/**
 * @brief Fixed-capacity bump-pointer arena.
 */
typedef struct Arena {
  /** Backing memory block. */
  uint8_t* base;
  /** Total capacity in bytes. */
  size_t cap;
  /** Current allocation offset. */
  size_t offset;
} Arena;

/**
 * @brief Allocate a new arena with the given capacity.
 *
 * @param capacity Total pool size in bytes (default ARENA_DEFAULT_CAPACITY for nodes).
 * @return Arena instance, or NULL on failure.
 */
Arena* arena_new(size_t capacity);

/**
 * @brief Allocate a block from the arena with 8-byte alignment.
 *
 * @param a Arena instance.
 * @param size Requested block size.
 * @return Aligned pointer within the arena, or NULL if insufficient space.
 */
void* arena_alloc(Arena* a, size_t size);

/**
 * @brief Reset the arena, making all memory available for reuse.
 *
 * Does NOT free the backing memory. Simply bumps the offset back to 0.
 *
 * @param a Arena instance. NULL is allowed.
 */
void arena_reset(Arena* a);

/**
 * @brief Free the arena and its backing memory.
 *
 * @param a Arena instance. NULL is allowed.
 */
void arena_free(Arena* a);

#endif /* MAGI_UTILS_ARENA_H */
