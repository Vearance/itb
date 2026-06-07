#define _POSIX_C_SOURCE 200809L

#include "hashmap.h"

#include "magi_error.h"

#include <limits.h>
#include <stdlib.h>
#include <string.h>

/**
 * @brief Compute a 64-bit FNV-1a hash for a null-terminated key string.
 *
 * Uses the FNV-1a (Fowler-Noll-Vo) non-cryptographic hash algorithm with
 * a 64-bit offset basis and prime multiplier. The result is used to index
 * into the open-addressing hash table via a power-of-two modulo.
 *
 * @param key Null-terminated string to hash.
 * @return 64-bit FNV-1a hash value.
 */
static size_t hash_key(const char* key) {
  const unsigned char* cursor = (const unsigned char*)key;
  size_t hash = 1469598103934665603ULL;

  while (*cursor != '\0') {
    hash ^= (size_t)(*cursor++);
    hash *= 1099511628211ULL;
  }

  return hash;
}

/**
 * @brief Insert a pre-owned key/value pair into a fresh bucket array.
 *
 * Used exclusively during rehashing to migrate entries from the old table
 * to a newly allocated larger table. The key pointer ownership is retained;
 * the caller must not free it separately. Linear probing resolves collisions.
 *
 * @param entries  The destination bucket array.
 * @param capacity Number of slots in the destination array.
 * @param key      Heap-allocated key string (already owned by the map).
 * @param value    Pointer to the associated value.
 * @return MAGI_OK on success, or MAGI_ERR_NOMEM on allocation failure.
 */
static int insert_existing(HashEntry* entries, size_t capacity, char* key, void* value) {
  size_t index = hash_key(key) & (capacity - 1U);

  while (entries[index].key != NULL) {
    index = (index + 1U) & (capacity - 1U);
  }

  entries[index].key = key;
  entries[index].value = value;
  entries[index].tombstone = false;
  return MAGI_OK;
}

/**
 * @brief Rehash the map into a larger bucket array.
 *
 * Allocates a new entry array of the requested capacity, then migrates all
 * live (non-tombstone, non-NULL) entries via insert_existing(). The old
 * array is freed on success. The tombstone counter is reset to zero after
 * migration.
 *
 * @param map          The hash map to resize.
 * @param new_capacity New capacity (must be a power of two).
 * @return MAGI_OK on success, or MAGI_ERR_NOMEM on allocation failure.
 */
static int resize_map(HashMap* map, size_t new_capacity) {
  HashEntry* new_entries = calloc(new_capacity, sizeof(HashEntry));

  if (new_entries == NULL) {
    magi_errno = MAGI_ERR_NOMEM;
    return MAGI_ERR_NOMEM;
  }

  for (size_t index = 0; index < map->capacity; ++index) {
    HashEntry* entry = &map->entries[index];

    if (entry->key != NULL && !entry->tombstone) {
      int status = insert_existing(new_entries, new_capacity, entry->key, entry->value);

      if (status != MAGI_OK) {
        free(new_entries);
        return status;
      }
    }
  }

  free(map->entries);
  map->entries = new_entries;
  map->capacity = new_capacity;
  map->tombstones = 0;
  return MAGI_OK;
}

HashMap* hashmap_new(size_t initial_capacity) {
  size_t capacity = 16;

  while (capacity < initial_capacity) {
    if (capacity > ((size_t)-1) / 2U) {
      magi_errno = MAGI_ERR_NOMEM;
      return NULL;
    }

    capacity <<= 1U;
  }

  HashMap* map = malloc(sizeof(*map));
  if (map == NULL) {
    magi_errno = MAGI_ERR_NOMEM;
    return NULL;
  }

  map->entries = calloc(capacity, sizeof(HashEntry));
  if (map->entries == NULL) {
    free(map);
    magi_errno = MAGI_ERR_NOMEM;
    return NULL;
  }

  map->capacity = capacity;
  map->count = 0;
  map->tombstones = 0;
  return map;
}

void hashmap_free(HashMap* map) {
  if (map == NULL) {
    return;
  }

  for (size_t index = 0; index < map->capacity; ++index) {
    free(map->entries[index].key);
  }

  free(map->entries);
  free(map);
}

int hashmap_set(HashMap* map, const char* key, void* value) {
  if (map == NULL || key == NULL) {
    magi_errno = MAGI_ERR_BADARGS;
    return MAGI_ERR_BADARGS;
  }

  if ((map->count + map->tombstones + 1U) * 10U > map->capacity * 7U) {
    int resize_status = resize_map(map, map->capacity * 2U);

    if (resize_status != MAGI_OK) {
      return resize_status;
    }
  }

  size_t index = hash_key(key) & (map->capacity - 1U);
  size_t first_tombstone = (size_t)-1;

  while (map->entries[index].key != NULL || map->entries[index].tombstone) {
    if (map->entries[index].key != NULL && strcmp(map->entries[index].key, key) == 0) {
      map->entries[index].value = value;
      return MAGI_OK;
    }

    if (map->entries[index].tombstone && first_tombstone == (size_t)-1) {
      first_tombstone = index;
    }

    index = (index + 1U) & (map->capacity - 1U);
  }

  size_t insert_index = (first_tombstone == (size_t)-1) ? index : first_tombstone;
  char* copied_key = strdup(key);

  if (copied_key == NULL) {
    magi_errno = MAGI_ERR_NOMEM;
    return MAGI_ERR_NOMEM;
  }

  map->entries[insert_index].key = copied_key;
  map->entries[insert_index].value = value;
  map->entries[insert_index].tombstone = false;
  map->count++;

  if (first_tombstone != (size_t)-1 && map->tombstones > 0U) {
    map->tombstones--;
  }

  return MAGI_OK;
}

void* hashmap_get(const HashMap* map, const char* key) {
  if (map == NULL || key == NULL) {
    return NULL;
  }

  size_t index = hash_key(key) & (map->capacity - 1U);

  while (map->entries[index].key != NULL || map->entries[index].tombstone) {
    if (map->entries[index].key != NULL && strcmp(map->entries[index].key, key) == 0) {
      return map->entries[index].value;
    }

    index = (index + 1U) & (map->capacity - 1U);
  }

  return NULL;
}

int hashmap_delete(HashMap* map, const char* key) {
  if (map == NULL || key == NULL) {
    magi_errno = MAGI_ERR_BADARGS;
    return MAGI_ERR_BADARGS;
  }

  size_t index = hash_key(key) & (map->capacity - 1U);

  while (map->entries[index].key != NULL || map->entries[index].tombstone) {
    if (map->entries[index].key != NULL && strcmp(map->entries[index].key, key) == 0) {
      free(map->entries[index].key);
      map->entries[index].key = NULL;
      map->entries[index].value = NULL;
      map->entries[index].tombstone = true;
      map->count--;
      map->tombstones++;
      return MAGI_OK;
    }

    index = (index + 1U) & (map->capacity - 1U);
  }

  magi_errno = MAGI_ERR_BADARGS;
  return MAGI_ERR_BADARGS;
}

void hashmap_foreach(const HashMap* map, void (*fn)(const char* key, void* value, void* ctx),
                     void* ctx) {
  if (map == NULL || fn == NULL) {
    return;
  }

  for (size_t index = 0; index < map->capacity; ++index) {
    if (map->entries[index].key != NULL && !map->entries[index].tombstone) {
      fn(map->entries[index].key, map->entries[index].value, ctx);
    }
  }
}