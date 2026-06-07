#define _POSIX_C_SOURCE 200809L

#include "arena.h"

#include <stdlib.h>
#include <string.h>

Arena* arena_new(size_t capacity) {
  if (capacity == 0U) {
    capacity = 65536U;
  }

  Arena* a = malloc(sizeof(*a));
  if (a == NULL) {
    return NULL;
  }

  a->base = calloc(1U, capacity);
  if (a->base == NULL) {
    free(a);
    return NULL;
  }

  a->cap = capacity;
  a->offset = 0U;
  return a;
}

void* arena_alloc(Arena* a, size_t size) {
  if (a == NULL || size == 0U) {
    return NULL;
  }

  /* 8-byte alignment */
  size_t aligned = (a->offset + 7U) & ~((size_t)7U);

  if (aligned + size > a->cap) {
    return NULL;
  }

  a->offset = aligned + size;
  return a->base + aligned;
}

void arena_reset(Arena* a) {
  if (a == NULL) {
    return;
  }

  a->offset = 0U;
}

void arena_free(Arena* a) {
  if (a == NULL) {
    return;
  }

  free(a->base);
  free(a);
}
