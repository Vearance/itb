#define _POSIX_C_SOURCE 200809L

#include "node_ops.h"

#include "layer2/host.h"
#include "layer2/switch.h"
#include "layer3/ipv4.h"
#include "layer3/router.h"
#include "layer4/l4_host.h"

#include <stdio.h>

typedef struct RouteForwardCtx {
  void (*fn)(const char* dest_cidr, const char* next_hop_ip, uint16_t out_port, void* ctx);
  void* ctx;
} RouteForwardCtx;

/**
 * @brief Forwarding callback for router_foreach_route().
 *
 * Converts a raw RoutingTableEntry into CIDR string format and invokes
 * the user-provided callback. If the next-hop address is all-zero ("0.0.0.0"),
 * it is reported as "direct" (directly connected route).
 *
 * @param route Routing table entry to forward.
 * @param ctx Opaque pointer to a RouteForwardCtx struct.
 */
static void forward_router_route(const RoutingTableEntry* route, void* ctx) {
  RouteForwardCtx* state = ctx;
  if (state == NULL || state->fn == NULL || route == NULL) {
    return;
  }

  char dest_cidr[32];
  char next_hop[16];
  if (ipv4_format_cidr(route->network, route->prefix_len, dest_cidr, sizeof(dest_cidr)) != 0) {
    return;
  }
  ipv4_address_to_string(route->next_hop, next_hop);
  state->fn(dest_cidr, ipv4_addr_is_zero(route->next_hop) ? "direct" : next_hop, route->out_port,
            state->ctx);
}

/**
 * @brief Create and initialize a node of the specified kind.
 *
 * For TOPOLOGY_NODE_HOST: creates a Host, then attaches IPv4 layer-3 and L4 host
 * support. On any failure the partially-allocated host is freed and NULL is returned.
 * For TOPOLOGY_NODE_SWITCH: creates a Switch and returns its Node pointer.
 * For TOPOLOGY_NODE_ROUTER: creates a Router and returns its Node pointer.
 *
 * @param kind The type of node to create (host, switch, or router).
 * @param name Unique node identifier.
 * @return Pointer to the new Node on success, NULL on failure.
 */
static Node* cli_create_node(TopologyNodeKind kind, const char* name) {
  switch (kind) {
  case TOPOLOGY_NODE_HOST: {
    Host* host = host_new(name);
    Node* node = host_as_node(host);
    if (node != NULL && ipv4_host_attach(node) != 0) {
      host_free(host);
      return NULL;
    }
    if (node != NULL && l4_host_attach(node) != 0) {
      host_free(host);
      return NULL;
    }
    return node;
  }
  case TOPOLOGY_NODE_SWITCH: {
    Switch* sw = switch_new(name);
    return switch_as_node(sw);
  }
  case TOPOLOGY_NODE_ROUTER: {
    Router* router = router_new(name);
    return router_as_node(router);
  }
  default:
    return NULL;
  }
}

/**
 * @brief Configure a host node with an IP address and default gateway.
 *
 * Delegates to host_configure().
 *
 * @param node Pointer to the host node to configure.
 * @param ip_address IPv4 address and prefix length (e.g. "192.168.1.10/24").
 * @param default_gateway Default gateway IPv4 address (e.g. "192.168.1.1").
 * @return MAGI_OK on success, otherwise an error code from host_configure().
 */
static int cli_configure_host(Node* node, const char* ip_address, const char* default_gateway) {
  return host_configure(host_from_node(node), ip_address, default_gateway);
}

/**
 * @brief Configure the number of ports on a switch node.
 *
 * Delegates to switch_configure_num_ports().
 *
 * @param node Pointer to the switch node to configure.
 * @param num_ports Total number of ports on the switch.
 * @return MAGI_OK on success, otherwise an error code.
 */
static int cli_configure_switch_num_ports(Node* node, uint16_t num_ports) {
  return switch_configure_num_ports(switch_from_node(node), num_ports);
}

/**
 * @brief Configure a single switch port with a VLAN mode and optional VLAN ID.
 *
 * Delegates to switch_configure_port().
 *
 * @param node Pointer to the switch node.
 * @param port Port number to configure.
 * @param mode_text Port mode string ("access" or "trunk").
 * @param vlan_id VLAN ID for access ports (ignored for trunk ports).
 * @return MAGI_OK on success, otherwise an error code.
 */
static int cli_configure_switch_port(Node* node, uint16_t port, const char* mode_text,
                                     uint16_t vlan_id) {
  return switch_configure_port(switch_from_node(node), port, mode_text, vlan_id);
}

/**
 * @brief Retrieve a switch port's current VLAN mode and VLAN ID.
 *
 * Delegates to switch_get_port_config().
 *
 * @param node Pointer to the switch node (const).
 * @param port Port number to query.
 * @param[out] mode_out Buffer to receive the mode string ("access" or "trunk"), or NULL.
 * @param mode_len Size of the mode_out buffer.
 * @param[out] vlan_id_out Pointer to receive the VLAN ID, or NULL.
 * @return true if the port configuration was retrieved successfully, false otherwise.
 */
static bool cli_get_switch_port_config(const Node* node, uint16_t port, char* mode_out,
                                       size_t mode_len, uint16_t* vlan_id_out) {
  SwitchPortConfig config = {0};
  if (!switch_get_port_config(switch_from_node_const(node), port, &config)) {
    return false;
  }

  if (mode_out != NULL && mode_len > 0U) {
    snprintf(mode_out, mode_len, "%s", config.mode == SWITCH_PORT_TRUNK ? "trunk" : "access");
  }
  if (vlan_id_out != NULL) {
    *vlan_id_out = config.vlan_id;
  }

  return true;
}

/**
 * @brief Add a route to a router node.
 *
 * Delegates to router_add_route().
 *
 * @param node Pointer to the router node.
 * @param dest_cidr Destination network in CIDR notation (e.g. "10.0.0.0/8").
 * @param next_hop_ip Next-hop IPv4 address, or "direct" for directly connected.
 * @param out_port Index of the egress interface.
 * @return MAGI_OK on success, otherwise an error code.
 */
static int cli_configure_router_route(Node* node, const char* dest_cidr, const char* next_hop_ip,
                                      uint16_t out_port) {
  return router_add_route(router_from_node(node), dest_cidr, next_hop_ip, out_port);
}

/**
 * @brief Iterate over all routes on a router node.
 *
 * Wraps router_foreach_route() and uses forward_router_route() to convert each
 * RoutingTableEntry into CIDR string format before passing to the callback.
 *
 * @param node Pointer to the router node (const).
 * @param fn Callback invoked for each route with dest_cidr, next_hop_ip, out_port, and ctx.
 * @param ctx Opaque user pointer forwarded to the callback.
 */
static void cli_foreach_router_route(const Node* node,
                                     void (*fn)(const char* dest_cidr, const char* next_hop_ip,
                                                uint16_t out_port, void* ctx),
                                     void* ctx) {
  RouteForwardCtx state = {.fn = fn, .ctx = ctx};
  router_foreach_route(router_from_node_const(node), forward_router_route, &state);
}

/**
 * @brief Return the concrete node-operation hooks for topology registries.
 *
 * The returned table provides callbacks that the topology module uses to
 * create, configure, and inspect nodes of all kinds. This is the CLI's
 * integration point with the topology layer.
 *
 * @return A stable pointer to a static TopologyNodeOps table.
 */
const TopologyNodeOps* cli_topology_node_ops(void) {
  static const TopologyNodeOps ops = {
      .create_node = cli_create_node,
      .configure_host = cli_configure_host,
      .configure_switch_num_ports = cli_configure_switch_num_ports,
      .configure_switch_port = cli_configure_switch_port,
      .get_switch_port_config = cli_get_switch_port_config,
      .configure_router_route = cli_configure_router_route,
      .foreach_router_route = cli_foreach_router_route,
  };

  return &ops;
}
