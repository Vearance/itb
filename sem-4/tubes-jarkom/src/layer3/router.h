/**
 * @file router.h
 * @brief Router node wrapper APIs.
 */

#ifndef MAGI_LAYER3_ROUTER_H
#define MAGI_LAYER3_ROUTER_H

#include <stddef.h>
#include <stdint.h>

#include "core/node.h"

/** @brief Opaque router specialization of Node. */
typedef struct Router Router;

typedef struct RoutingTableEntry {
  uint8_t network[4];
  uint8_t mask[4];
  int prefix_len;
  uint8_t next_hop[4];
  uint16_t out_port;
  uint8_t metric;
} RoutingTableEntry;

typedef void (*router_route_visitor_fn)(const RoutingTableEntry* route, void* ctx);

/**
 * @brief Create a router node.
 *
 * @param name Router name.
 * @return Router instance, or NULL on failure.
 */
Router* router_new(const char* name);

/**
 * @brief Destroy a router node.
 *
 * @param router Router to destroy. NULL is allowed.
 */
void router_free(Router* router);

/**
 * @brief View a Router as its embedded generic Node.
 *
 * @param router Router instance.
 * @return Generic node pointer, or NULL.
 */
Node* router_as_node(Router* router);

/**
 * @brief View a Router as its embedded generic Node.
 *
 * @param router Router instance.
 * @return Generic node pointer, or NULL.
 */
const Node* router_as_node_const(const Router* router);

/**
 * @brief View a generic Node known to be a router as Router.
 *
 * @param node Generic node.
 * @return Router pointer, or NULL.
 */
Router* router_from_node(Node* node);

/**
 * @brief View a generic Node known to be a router as Router.
 *
 * @param node Generic node.
 * @return Router pointer, or NULL.
 */
const Router* router_from_node_const(const Node* node);

int router_add_route(Router* router, const char* dest_cidr, const char* next_hop_ip,
                     uint16_t out_port);
int router_remove_route(Router* router, const char* dest_cidr);
const RoutingTableEntry* lpm_lookup(Router* router, const uint8_t dst_ip[4]);
void router_handle_receive(Node* node, struct Interface* in_iface, const uint8_t* data, size_t len);
void router_foreach_route(const Router* router, router_route_visitor_fn fn, void* ctx);
void router_print_routes(const Router* router);

/**
 * @brief Print the router's ARP cache contents.
 *
 * Iterates the ARP cache hashmap and logs each IP-to-MAC mapping.
 * If the cache is empty, logs "ARP cache empty".
 *
 * @param router The router whose ARP cache to display.
 */
void router_print_arp_cache(const Router* router);

#endif
