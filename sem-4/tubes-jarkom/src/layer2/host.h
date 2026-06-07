/**
 * @file host.h
 * @brief Host node wrapper APIs.
 */

#ifndef MAGI_LAYER2_HOST_H
#define MAGI_LAYER2_HOST_H

#include <stddef.h>
#include <stdint.h>

#include "core/node.h"

/** @brief Opaque host specialization of Node. */
typedef struct Host Host;

/**
 * @brief Create a host node.
 *
 * @param name Host name.
 * @return Host instance, or NULL on failure.
 */
Host* host_new(const char* name);

/**
 * @brief Destroy a host node.
 *
 * @param host Host to destroy. NULL is allowed.
 */
void host_free(Host* host);

/**
 * @brief View a Host as its embedded generic Node.
 *
 * @param host Host instance.
 * @return Generic node pointer, or NULL.
 */
Node* host_as_node(Host* host);

/**
 * @brief View a Host as its embedded generic Node.
 *
 * @param host Host instance.
 * @return Generic node pointer, or NULL.
 */
const Node* host_as_node_const(const Host* host);

/**
 * @brief View a generic Node known to be a host as Host.
 *
 * @param node Generic node.
 * @return Host pointer, or NULL.
 */
Host* host_from_node(Node* node);

/**
 * @brief View a generic Node known to be a host as Host.
 *
 * @param node Generic node.
 * @return Host pointer, or NULL.
 */
const Host* host_from_node_const(const Node* node);

/**
 * @brief Configure host IP metadata loaded from topology JSON.
 *
 * @param host Host node.
 * @param ip_address Host CIDR string.
 * @param default_gateway Default gateway string.
 * @return MAGI_OK on success, otherwise an error code.
 */
int host_configure(Host* host, const char* ip_address, const char* default_gateway);

/**
 * @brief Send an L3 payload through the host's L2 path, resolving ARP if needed.
 *
 * @param host Host node.
 * @param target_ip Dotted IPv4 destination.
 * @param ethertype Ethernet ethertype for the payload.
 * @param payload Payload bytes.
 * @param payload_len Payload length.
 * @return MAGI_OK on success, otherwise an error code.
 */
int host_send_l3_packet(Host* host, const char* target_ip, uint16_t ethertype,
                        const uint8_t* payload, size_t payload_len);

/**
 * @brief Trigger a small Milestone-1 ARP probe for CLI ping scaffolding.
 *
 * @param host Host node.
 * @param target_ip Dotted IPv4 destination.
 * @return MAGI_OK on success, otherwise an error code.
 */
int host_probe_l2(Host* host, const char* target_ip);

/**
 * @brief Print a host ARP cache.
 *
 * @param host Host node.
 */
void host_print_arp_cache(const Host* host);

#endif
