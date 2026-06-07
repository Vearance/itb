/**
 * @file hashmap.h
 * @brief Open-addressing hash map utilities used across MAGI.
 */

#ifndef MAGI_UTILS_HASHMAP_H
#define MAGI_UTILS_HASHMAP_H

#include <stdbool.h>
#include <stddef.h>

/**
 * @brief One hash table slot.
 */
typedef struct HashEntry {
  /** Heap-allocated key string. */
  char* key;
  /** Opaque value pointer. */
  void* value;
  /** Tombstone marker used after deletes. */
  bool tombstone;
} HashEntry;

/**
 * @brief Hash map state.
 */
typedef struct HashMap {
  /** Slot array. */
  HashEntry* entries;
  /** Number of slots (power of two). */
  size_t capacity;
  /** Number of live entries. */
  size_t count;
  /** Number of tombstone slots. */
  size_t tombstones;
} HashMap;

/**
 * @brief Create a new hash map.
 *
 * @param initial_capacity Minimum desired capacity.
 * @return Hash map instance, or NULL on failure.
 */
HashMap* hashmap_new(size_t initial_capacity);

/**
 * @brief Destroy a hash map and free key storage.
 *
 * @param map Hash map to destroy. NULL is allowed.
 */
void hashmap_free(HashMap* map);

/**
 * @brief Insert or update a key/value pair.
 *
 * @param map Hash map instance.
 * @param key Lookup key.
 * @param value Opaque value pointer.
 * @return MAGI_OK on success, otherwise an error code.
 */
int hashmap_set(HashMap* map, const char* key, void* value);

/**
 * @brief Look up a value by key.
 *
 * @param map Hash map instance.
 * @param key Lookup key.
 * @return Stored value, or NULL if key is absent.
 */
void* hashmap_get(const HashMap* map, const char* key);

/**
 * @brief Delete one key from the map.
 *
 * @param map Hash map instance.
 * @param key Key to remove.
 * @return MAGI_OK on success, otherwise an error code.
 */
int hashmap_delete(HashMap* map, const char* key);

/**
 * @brief Iterate over all live entries.
 *
 * @param map Hash map instance.
 * @param fn Callback invoked for each entry.
 * @param ctx Opaque callback context pointer.
 */
void hashmap_foreach(const HashMap* map, void (*fn)(const char* key, void* value, void* ctx),
                     void* ctx);

#endif