#define _POSIX_C_SOURCE 200809L

#include "mac.h"

#include "magi_error.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/**
 * @brief Compute a deterministic 32-bit FNV-1a hash for a string.
 *
 * Implements the FNV-1a non-cryptographic hash with a 32-bit offset basis
 * and prime multiplier. Used by mac_generate() to produce deterministic
 * MAC addresses from node names. Returns a fixed value when text is NULL.
 *
 * @param text Null-terminated string to hash. If NULL, returns the
 *             offset basis value (2166136261) unchanged.
 * @return 32-bit FNV-1a hash value.
 */
static uint32_t fnv1a_32(const char* text) {
  uint32_t hash = 2166136261u;

  if (text == NULL) {
    return hash;
  }

  for (const unsigned char* cursor = (const unsigned char*)text; *cursor != '\0'; ++cursor) {
    hash ^= (uint32_t)(*cursor);
    hash *= 16777619u;
  }

  return hash;
}

void mac_generate(uint8_t out[6], const char* node_name, uint16_t port) {
  if (out == NULL) {
    return;
  }

  uint64_t state = ((uint64_t)fnv1a_32(node_name) << 16U) ^ (uint64_t)port ^ 0x9E3779B97F4A7C15ULL;

  for (size_t index = 0; index < 6U; ++index) {
    state = state * 6364136223846793005ULL + 1442695040888963407ULL;
    out[index] = (uint8_t)(state >> 56U);
  }

  out[0] = (uint8_t)((out[0] | 0x02U) & 0xFEU);
}

void mac_to_str(const uint8_t mac[6], char out[18]) {
  if (out == NULL) {
    return;
  }

  if (mac == NULL) {
    memset(out, 0, 18U);
    return;
  }

  snprintf(out, 18U, "%02X:%02X:%02X:%02X:%02X:%02X", mac[0], mac[1], mac[2], mac[3], mac[4],
           mac[5]);
}

int mac_from_str(const char* str, uint8_t out[6]) {
  if (str == NULL || out == NULL || strlen(str) != 17U) {
    magi_errno = MAGI_ERR_BADARGS;
    return MAGI_ERR_BADARGS;
  }

  unsigned int octets[6] = {0};
  int parsed = sscanf(str, "%2x:%2x:%2x:%2x:%2x:%2x", &octets[0], &octets[1], &octets[2],
                      &octets[3], &octets[4], &octets[5]);

  if (parsed != 6) {
    magi_errno = MAGI_ERR_BADARGS;
    return MAGI_ERR_BADARGS;
  }

  for (size_t index = 0; index < 6U; ++index) {
    if (octets[index] > 0xFFU) {
      magi_errno = MAGI_ERR_BADARGS;
      return MAGI_ERR_BADARGS;
    }

    out[index] = (uint8_t)octets[index];
  }

  return MAGI_OK;
}