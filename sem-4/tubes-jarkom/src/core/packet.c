#define _POSIX_C_SOURCE 200809L

#include "packet.h"

#include <stdlib.h>

/**
 * @brief Free a packet and its underlying buffer.
 *
 * Releases the memory allocated for the packet's buffer (self->buf) and the
 * Packet struct itself. This is the default destroy callback used when a
 * concrete protocol type does not supply its own custom destroy function.
 * Safe to call with NULL.
 *
 * @param self The packet to destroy. May be NULL.
 */
void packet_default_destroy(Packet* self) {
  if (self == NULL) {
    return;
  }

  free(self->buf);
  free(self);
}