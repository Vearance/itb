/**
 * @file topology.h
 * @brief In-memory topology model and mutation/query APIs.
 */

#ifndef MAGI_TOPOLOGY_TOPOLOGY_H
#define MAGI_TOPOLOGY_TOPOLOGY_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "core/link.h"
#include "core/node.h"

/**
 * @brief Supported node categories in the topology.
 */
typedef enum TopologyNodeKind {
  TOPOLOGY_NODE_HOST,
  TOPOLOGY_NODE_SWITCH,
  TOPOLOGY_NODE_ROUTER
} TopologyNodeKind;

/**
 * @brief Layer-specific node hooks supplied by a top-level integrator.
 *
 * topology/ owns the registry and JSON metadata, but concrete host/switch/router
 * construction lives outside this module to keep include dependencies downward.
 */
typedef struct TopologyNodeOps {
  Node* (*create_node)(TopologyNodeKind kind, const char* name);
  int (*configure_host)(Node* node, const char* ip_address, const char* default_gateway);
  int (*configure_switch_num_ports)(Node* node, uint16_t num_ports);
  int (*configure_switch_port)(Node* node, uint16_t port, const char* mode_text, uint16_t vlan_id);
  bool (*get_switch_port_config)(const Node* node, uint16_t port, char* mode_out, size_t mode_len,
                                 uint16_t* vlan_id_out);
  int (*configure_router_route)(Node* node, const char* dest_cidr, const char* next_hop_ip,
                                uint16_t out_port);
  void (*foreach_router_route)(const Node* node,
                               void (*fn)(const char* dest_cidr, const char* next_hop_ip,
                                          uint16_t out_port, void* ctx),
                               void* ctx);
} TopologyNodeOps;

/**
 * @brief Stored metadata for one node.
 */
typedef struct TopologyNodeInfo {
  /** Node category. */
  TopologyNodeKind kind;
  /** Owned node object. */
  Node* node;
  /** Optional host CIDR address preserved from topology JSON. */
  char ip_address[64];
  /** Optional host default gateway preserved from topology JSON. */
  char default_gateway[64];
  /** Declared switch port count, if known. */
  uint16_t num_ports;
} TopologyNodeInfo;

/**
 * @brief Stored metadata for one link.
 */
typedef struct TopologyLinkInfo {
  /** Owned link object. */
  Link* link;
  /** Endpoint A node name. */
  char node_a[64];
  /** Endpoint A port. */
  uint16_t port_a;
  /** Endpoint B node name. */
  char node_b[64];
  /** Endpoint B port. */
  uint16_t port_b;
} TopologyLinkInfo;

/**
 * @brief Top-level topology registry.
 */
typedef struct Topology {
  /** Map from node name to TopologyNodeInfo. */
  HashMap* nodes;
  /** Map from normalized link key to TopologyLinkInfo. */
  HashMap* links;
  /** Default load/save path. */
  char* source_path;
  /** Concrete node hooks supplied by CLI/main. */
  const TopologyNodeOps* node_ops;
} Topology;

/**
 * @brief Allocate and initialize an empty topology.
 *
 * @return Topology instance, or NULL on failure.
 */
Topology* topology_new(void);

/**
 * @brief Register concrete node hooks for this topology.
 *
 * @param topology Mutable topology.
 * @param ops Stable callback table. The caller retains ownership.
 */
void topology_set_node_ops(Topology* topology, const TopologyNodeOps* ops);

/**
 * @brief Destroy a topology and all owned nodes/links.
 *
 * @param topology Topology to destroy. NULL is allowed.
 */
void topology_free(Topology* topology);

/**
 * @brief Convert node kind enum to printable name.
 *
 * @param kind Node kind.
 * @return Constant kind name string.
 */
const char* topology_kind_name(TopologyNodeKind kind);

/**
 * @brief Parse node kind text (host/switch/router).
 *
 * @param kind_text Input text.
 * @return Parsed kind, or (TopologyNodeKind)-1 if invalid.
 */
TopologyNodeKind topology_parse_kind(const char* kind_text);

/**
 * @brief Add a node to the topology.
 *
 * @param topology Mutable topology.
 * @param kind Node category.
 * @param name Unique node name.
 * @return Node metadata on success, or NULL on failure.
 */
TopologyNodeInfo* topology_add_node(Topology* topology, TopologyNodeKind kind, const char* name);

/**
 * @brief Configure host L2/IP metadata after loading topology JSON.
 *
 * @param topology Mutable topology.
 * @param name Host name.
 * @param ip_address Host CIDR string.
 * @param default_gateway Default gateway string.
 * @return MAGI_OK on success, otherwise an error code.
 */
int topology_configure_host(Topology* topology, const char* name, const char* ip_address,
                            const char* default_gateway);

/**
 * @brief Ensure a switch has the requested declared number of ports.
 *
 * @param topology Mutable topology.
 * @param name Switch name.
 * @param num_ports Port count.
 * @return MAGI_OK on success, otherwise an error code.
 */
int topology_configure_switch_num_ports(Topology* topology, const char* name, uint16_t num_ports);

/**
 * @brief Configure one switch port VLAN mode.
 *
 * @param topology Mutable topology.
 * @param name Switch name.
 * @param port Port number.
 * @param mode_text "access" or "trunk".
 * @param vlan_id Access VLAN id.
 * @return MAGI_OK on success, otherwise an error code.
 */
int topology_configure_switch_port(Topology* topology, const char* name, uint16_t port,
                                   const char* mode_text, uint16_t vlan_id);

/**
 * @brief Fetch an explicit switch port VLAN config for serialization.
 *
 * @param topology Topology instance.
 * @param name Switch name.
 * @param port Port number.
 * @param mode_out Optional destination for mode text.
 * @param mode_len Destination mode buffer length.
 * @param vlan_id_out Optional destination for VLAN id.
 * @return true when an explicit config exists.
 */
bool topology_get_switch_port_config(const Topology* topology, const char* name, uint16_t port,
                                     char* mode_out, size_t mode_len, uint16_t* vlan_id_out);

/**
 * @brief Fetch node metadata by name.
 *
 * @param topology Topology instance.
 * @param name Node name.
 * @return Node metadata, or NULL if missing.
 */
TopologyNodeInfo* topology_get_node_info(const Topology* topology, const char* name);

/**
 * @brief Fetch node object by name.
 *
 * @param topology Topology instance.
 * @param name Node name.
 * @return Node object, or NULL if missing.
 */
Node* topology_get_node(const Topology* topology, const char* name);

/**
 * @brief Count nodes of a given kind.
 *
 * @param topology Topology instance.
 * @param kind Node category to count.
 * @return Number of matching nodes.
 */
size_t topology_count_nodes_of_kind(const Topology* topology, TopologyNodeKind kind);

/**
 * @brief Add a point-to-point link between two node ports.
 *
 * @param topology Mutable topology.
 * @param node_a Endpoint A node name.
 * @param port_a Endpoint A port.
 * @param node_b Endpoint B node name.
 * @param port_b Endpoint B port.
 * @param delay_ms Link delay in milliseconds.
 * @param mtu Link MTU in bytes.
 * @return Link metadata on success, or NULL on failure.
 */
TopologyLinkInfo* topology_add_link(Topology* topology, const char* node_a, uint16_t port_a,
                                    const char* node_b, uint16_t port_b, uint32_t delay_ms,
                                    uint16_t mtu);

/**
 * @brief Remove a point-to-point link between two node ports.
 *
 * @param topology Mutable topology.
 * @param node_a Endpoint A node name.
 * @param port_a Endpoint A port.
 * @param node_b Endpoint B node name.
 * @param port_b Endpoint B port.
 * @return MAGI_OK on success, otherwise an error code.
 */
int topology_remove_link(Topology* topology, const char* node_a, uint16_t port_a,
                         const char* node_b, uint16_t port_b);

/**
 * @brief Count links in the topology.
 *
 * @param topology Topology instance.
 * @return Number of links.
 */
size_t topology_count_links(const Topology* topology);

/**
 * @brief Print a human-readable topology summary.
 *
 * @param topology Topology instance.
 */
void topology_print(const Topology* topology);

/**
 * @brief Replace destination topology state with source state.
 *
 * Ownership of internal maps and source path is transferred from source to
 * destination. The source topology is left empty after success.
 *
 * @param destination Topology that will receive source state.
 * @param source Topology that donates its state.
 * @return MAGI_OK on success, otherwise an error code.
 */
int topology_replace_contents(Topology* destination, Topology* source);

#endif
