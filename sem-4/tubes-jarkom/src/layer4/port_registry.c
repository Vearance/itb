#define _POSIX_C_SOURCE 200809L

#include "port_registry.h"

#include "utils/magi_error.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/**
 * @brief Build a registry key string for a protocol/port pair.
 *
 * Produces "tcp:<port>" or "udp:<port>" depending on protocol.
 *
 * @param protocol IPPROTO_TCP (6) or IPPROTO_UDP (17).
 * @param port     Port number.
 * @param out      Destination buffer (must hold at least 16 chars).
 */
void port_registry_key(uint8_t protocol, uint16_t port, char out[16]) {
  const char* proto_name = (protocol == PORT_PROTOCOL_TCP) ? "tcp" : "udp";
  snprintf(out, 16U, "%s:%u", proto_name, (unsigned)port);
}

/**
 * @brief Create a new empty port registry.
 *
 * Allocates a HashMap with an initial capacity of 16.
 *
 * @return New HashMap pointer, or NULL on allocation failure.
 */
HashMap* port_registry_new(void) {
  return hashmap_new(16U);
}

/**
 * @brief Bind a socket (or opaque handler) to a protocol/port.
 *
 * Creates a PortBinding entry and inserts it into the registry.
 * Returns MAGI_ERR_PORTUSED if the protocol:port pair is already bound.
 *
 * @param reg      Port registry.
 * @param protocol IPPROTO_TCP (6) or IPPROTO_UDP (17).
 * @param port     Port number.
 * @param socket   Opaque socket or handler pointer to bind.
 * @return MAGI_OK on success, MAGI_ERR_PORTUSED if already bound,
 *         MAGI_ERR_NOMEM on allocation failure, MAGI_ERR_BADARGS on null input.
 */
int port_registry_bind(HashMap* reg, uint8_t protocol, uint16_t port, void* socket) {
  if (reg == NULL || socket == NULL) {
    magi_errno = MAGI_ERR_BADARGS;
    return MAGI_ERR_BADARGS;
  }

  char key[16];
  port_registry_key(protocol, port, key);

  if (hashmap_get(reg, key) != NULL) {
    magi_errno = MAGI_ERR_PORTUSED;
    return MAGI_ERR_PORTUSED;
  }

  PortBinding* binding = malloc(sizeof(*binding));
  if (binding == NULL) {
    magi_errno = MAGI_ERR_NOMEM;
    return MAGI_ERR_NOMEM;
  }

  binding->protocol = protocol;
  binding->port = port;
  binding->socket = socket;

  int status = hashmap_set(reg, key, binding);
  if (status != MAGI_OK) {
    free(binding);
    return status;
  }

  return MAGI_OK;
}

/**
 * @brief Look up a binding by protocol and port.
 *
 * Searches the registry for the given protocol:port pair and returns
 * the associated socket pointer if found.
 *
 * @param reg      Port registry.
 * @param protocol IPPROTO_TCP (6) or IPPROTO_UDP (17).
 * @param port     Port number.
 * @return Socket pointer if found, NULL if not bound or registry is NULL.
 */
void* port_registry_lookup(HashMap* reg, uint8_t protocol, uint16_t port) {
  if (reg == NULL) {
    return NULL;
  }

  char key[16];
  port_registry_key(protocol, port, key);

  PortBinding* binding = hashmap_get(reg, key);
  if (binding == NULL) {
    return NULL;
  }

  return binding->socket;
}

/**
 * @brief Remove a binding from the registry.
 *
 * Looks up the protocol:port pair, removes it from the hashmap,
 * and frees the associated PortBinding struct.
 *
 * @param reg      Port registry.
 * @param protocol IPPROTO_TCP (6) or IPPROTO_UDP (17).
 * @param port     Port number.
 * @return MAGI_OK on success, MAGI_ERR_BADARGS if not found or registry is NULL.
 */
int port_registry_unbind(HashMap* reg, uint8_t protocol, uint16_t port) {
  if (reg == NULL) {
    magi_errno = MAGI_ERR_BADARGS;
    return MAGI_ERR_BADARGS;
  }

  char key[16];
  port_registry_key(protocol, port, key);

  PortBinding* binding = hashmap_get(reg, key);
  if (binding == NULL) {
    magi_errno = MAGI_ERR_BADARGS;
    return MAGI_ERR_BADARGS;
  }

  int status = hashmap_delete(reg, key);
  free(binding);
  return status;
}

/**
 * @brief Free a PortBinding entry (hashmap_foreach callback).
 *
 * Frees the PortBinding struct allocated during port_registry_bind.
 * Does NOT free the socket itself — socket lifecycle is managed
 * by the caller.
 *
 * @param key   Registry key (unused).
 * @param value PortBinding pointer to free.
 * @param ctx   Unused context pointer.
 */
void port_registry_free_binding(const char* key, void* value, void* ctx) {
  (void)key;
  (void)ctx;
  free(value);
}
