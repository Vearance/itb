/**
 * @file port_registry.h
 * @brief Port-to-socket binding registry using the generic HashMap.
 *
 * Key format: "tcp:<port>" or "udp:<port>" → PortBinding*
 */

#ifndef MAGI_LAYER4_PORT_REGISTRY_H
#define MAGI_LAYER4_PORT_REGISTRY_H

#include <stdint.h>

#include "utils/hashmap.h"

#define PORT_PROTOCOL_TCP 6U
#define PORT_PROTOCOL_UDP 17U

/**
 * @brief One port binding entry.
 */
typedef struct PortBinding {
  uint8_t protocol; /* IPPROTO_TCP or IPPROTO_UDP */
  uint16_t port;
  void* socket; /* TCPSocket* or UDPSocket* */
} PortBinding;

/**
 * @brief Create a new empty port registry (HashMap).
 *
 * @return New HashMap, or NULL on failure.
 */
HashMap* port_registry_new(void);

/**
 * @brief Build a registry key string for a protocol/port pair.
 *
 * @param protocol IPPROTO_TCP or IPPROTO_UDP.
 * @param port     Port number.
 * @param out      Destination buffer (must hold at least 16 chars).
 */
void port_registry_key(uint8_t protocol, uint16_t port, char out[16]);

/**
 * @brief Bind a socket to a protocol/port.
 *
 * @param reg      Port registry.
 * @param protocol IPPROTO_TCP or IPPROTO_UDP.
 * @param port     Port number.
 * @param socket   Opaque socket pointer.
 * @return MAGI_OK on success, MAGI_ERR_PORTUSED if already bound.
 */
int port_registry_bind(HashMap* reg, uint8_t protocol, uint16_t port, void* socket);

/**
 * @brief Look up a binding by protocol/port.
 *
 * @param reg      Port registry.
 * @param protocol IPPROTO_TCP or IPPROTO_UDP.
 * @param port     Port number.
 * @return Socket pointer, or NULL if not bound.
 */
void* port_registry_lookup(HashMap* reg, uint8_t protocol, uint16_t port);

/**
 * @brief Remove a binding.
 *
 * @param reg      Port registry.
 * @param protocol IPPROTO_TCP or IPPROTO_UDP.
 * @param port     Port number.
 * @return MAGI_OK on success, MAGI_ERR_BADARGS if not found.
 */
int port_registry_unbind(HashMap* reg, uint8_t protocol, uint16_t port);

/**
 * @brief Free a port binding (callback for hashmap_foreach-free).
 *
 * Frees the PortBinding struct. Does NOT free the socket itself.
 *
 * @param key   Registry key.
 * @param value PortBinding pointer.
 * @param ctx   Unused context.
 */
void port_registry_free_binding(const char* key, void* value, void* ctx);

#endif /* MAGI_LAYER4_PORT_REGISTRY_H */
