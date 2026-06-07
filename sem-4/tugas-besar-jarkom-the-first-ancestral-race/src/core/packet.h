/**
 * @file packet.h
 * @brief Base packet interface used by protocol-specific packet types.
 */

#ifndef MAGI_CORE_PACKET_H
#define MAGI_CORE_PACKET_H

#include <stddef.h>
#include <stdint.h>

/**
 * @brief Generic packet object with virtual-like operations.
 */
typedef struct Packet {
  /** Backing serialized packet bytes. */
  uint8_t* buf;
  /** Number of bytes in buf. */
  size_t len;
  /** Serialize packet state into a caller-provided buffer. */
  int (*to_bytes)(struct Packet* self, uint8_t* out, size_t out_len);
  /** Parse packet state from a serialized byte buffer. */
  int (*from_bytes)(struct Packet* self, const uint8_t* in, size_t in_len);
  /** Destroy function for packet-specific cleanup. */
  void (*destroy)(struct Packet* self);
} Packet;

/**
 * @brief Default packet destructor for heap-allocated packet buffers.
 *
 * @param self Packet instance to destroy. NULL is allowed.
 */
void packet_default_destroy(Packet* self);

#endif