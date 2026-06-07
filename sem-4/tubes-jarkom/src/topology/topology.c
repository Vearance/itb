#define _POSIX_C_SOURCE 200809L

#include "topology.h"

#include "core/interface.h"
#include "utils/log.h"
#include "utils/magi_error.h"

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>

typedef struct NodeCollectState {
  TopologyNodeInfo** items;
  TopologyNodeKind kind;
  size_t count;
  size_t index;
} NodeCollectState;

typedef struct LinkCollectState {
  TopologyLinkInfo** items;
  size_t count;
  size_t index;
} LinkCollectState;

typedef struct CountKindState {
  TopologyNodeKind kind;
  size_t count;
} CountKindState;

/**
 * @brief Count nodes that match a requested kind during map iteration.
 */
static void count_kind_cb(const char* key, void* value, void* ctx) {
  (void)key;

  CountKindState* counter_state = ctx;
  TopologyNodeInfo* info = value;
  if (info != NULL && info->kind == counter_state->kind) {
    counter_state->count++;
  }
}

/**
 * @brief Duplicate a string onto the heap.
 */
static char* duplicate_string(const char* text) {
  if (text == NULL) {
    return NULL;
  }

  size_t len = strlen(text) + 1U;
  char* copy = malloc(len);
  if (copy == NULL) {
    return NULL;
  }

  memcpy(copy, text, len);
  return copy;
}

/**
 * @brief Release a stored node metadata entry.
 */
static void free_node_info(const char* key, void* value, void* ctx) {
  (void)key;
  (void)ctx;

  TopologyNodeInfo* info = value;
  if (info != NULL) {
    node_free(info->node);
    free(info);
  }
}

/**
 * @brief Release a stored link metadata entry.
 */
static void free_link_info(const char* key, void* value, void* ctx) {
  (void)key;
  (void)ctx;

  TopologyLinkInfo* info = value;
  if (info != NULL) {
    link_free(info->link);
    free(info);
  }
}

/**
 * @brief Collect nodes of the requested kind into a scratch array.
 */
static void collect_node_info(const char* key, void* value, void* ctx) {
  (void)key;

  NodeCollectState* state = ctx;
  TopologyNodeInfo* info = value;

  if (info != NULL && info->kind == state->kind && state->index < state->count) {
    state->items[state->index++] = info;
  }
}

/**
 * @brief Collect link metadata entries into a scratch array.
 */
static void collect_link_info(const char* key, void* value, void* ctx) {
  (void)key;

  LinkCollectState* state = ctx;
  state->items[state->index++] = value;
}

/**
 * @brief Compare node metadata for deterministic ordering.
 */
static int compare_node_info(const void* lhs, const void* rhs) {
  const TopologyNodeInfo* left = *(const TopologyNodeInfo* const*)lhs;
  const TopologyNodeInfo* right = *(const TopologyNodeInfo* const*)rhs;

  if (left->kind != right->kind) {
    return (int)left->kind - (int)right->kind;
  }

  return strcmp(left->node->name, right->node->name);
}

/**
 * @brief Compare link metadata for deterministic ordering.
 */
static int compare_link_info(const void* lhs, const void* rhs) {
  const TopologyLinkInfo* left = *(const TopologyLinkInfo* const*)lhs;
  const TopologyLinkInfo* right = *(const TopologyLinkInfo* const*)rhs;

  int comparison = strcmp(left->node_a, right->node_a);
  if (comparison != 0) {
    return comparison;
  }

  if (left->port_a != right->port_a) {
    return (int)left->port_a - (int)right->port_a;
  }

  comparison = strcmp(left->node_b, right->node_b);
  if (comparison != 0) {
    return comparison;
  }

  if (left->port_b != right->port_b) {
    return (int)left->port_b - (int)right->port_b;
  }

  return 0;
}

/**
 * @brief Build a normalized key for a bidirectional link.
 */
static int build_link_key(const char* node_a, uint16_t port_a, const char* node_b, uint16_t port_b,
                          char* out, size_t out_len) {
  if (node_a == NULL || node_b == NULL || out == NULL || out_len == 0U) {
    return MAGI_ERR_BADARGS;
  }

  char left[80];
  char right[80];

  snprintf(left, sizeof(left), "%s:%u", node_a, (unsigned)port_a);
  snprintf(right, sizeof(right), "%s:%u", node_b, (unsigned)port_b);

  const char* first = left;
  const char* second = right;

  if (strcmp(left, right) > 0) {
    first = right;
    second = left;
  }

  int written = snprintf(out, out_len, "%s|%s", first, second);
  if (written < 0 || (size_t)written >= out_len) {
    return MAGI_ERR_BADARGS;
  }

  return MAGI_OK;
}

/**
 * @brief Allocate and initialize an empty topology.
 *
 * Creates a new Topology instance with empty nodes and links hash maps,
 * and sets the default source path to "topology.json".
 *
 * @return Pointer to the new Topology on success, or NULL on allocation failure.
 */
Topology* topology_new(void) {
  Topology* topology = calloc(1, sizeof(*topology));
  if (topology == NULL) {
    magi_errno = MAGI_ERR_NOMEM;
    return NULL;
  }

  topology->nodes = hashmap_new(16U);
  topology->links = hashmap_new(16U);
  topology->source_path = duplicate_string("topology.json");

  if (topology->nodes == NULL || topology->links == NULL || topology->source_path == NULL) {
    topology_free(topology);
    magi_errno = MAGI_ERR_NOMEM;
    return NULL;
  }

  return topology;
}

/**
 * @brief Register concrete node hooks for this topology.
 *
 * The ops table provides callbacks for creating and configuring nodes
 * (host, switch, router). The caller retains ownership of the ops pointer.
 *
 * @param topology Mutable topology instance.
 * @param ops Stable callback table; may be NULL to clear.
 */
void topology_set_node_ops(Topology* topology, const TopologyNodeOps* ops) {
  if (topology != NULL) {
    topology->node_ops = ops;
  }
}

/**
 * @brief Destroy a topology and all its owned resources.
 *
 * Frees every link (via link_free) and every node (via node_free) owned
 * by the topology, then releases the hash maps and the topology struct
 * itself. Safe to call with a NULL pointer.
 *
 * @param topology Topology to destroy, or NULL.
 */
void topology_free(Topology* topology) {
  if (topology == NULL) {
    return;
  }

  if (topology->links != NULL) {
    hashmap_foreach(topology->links, free_link_info, NULL);
    hashmap_free(topology->links);
  }

  if (topology->nodes != NULL) {
    hashmap_foreach(topology->nodes, free_node_info, NULL);
    hashmap_free(topology->nodes);
  }

  free(topology->source_path);
  free(topology);
}

/**
 * @brief Convert a node kind enum to its printable name.
 *
 * @param kind Node kind value.
 * @return Constant string "HOST", "SWITCH", "ROUTER", or "UNKNOWN" for
 *         unrecognised values.
 */
const char* topology_kind_name(TopologyNodeKind kind) {
  switch (kind) {
  case TOPOLOGY_NODE_HOST:
    return "HOST";
  case TOPOLOGY_NODE_SWITCH:
    return "SWITCH";
  case TOPOLOGY_NODE_ROUTER:
    return "ROUTER";
  default:
    return "UNKNOWN";
  }
}

/**
 * @brief Parse a node kind from its textual representation.
 *
 * Accepts "host", "switch", or "router" (case-insensitive).
 *
 * @param kind_text Input text to parse.
 * @return Parsed TopologyNodeKind on success, or (TopologyNodeKind)-1 if
 *         the text is NULL or does not match any known kind.
 */
TopologyNodeKind topology_parse_kind(const char* kind_text) {
  if (kind_text == NULL) {
    return (TopologyNodeKind)-1;
  }

  if (strcasecmp(kind_text, "host") == 0) {
    return TOPOLOGY_NODE_HOST;
  }

  if (strcasecmp(kind_text, "switch") == 0) {
    return TOPOLOGY_NODE_SWITCH;
  }

  if (strcasecmp(kind_text, "router") == 0) {
    return TOPOLOGY_NODE_ROUTER;
  }

  return (TopologyNodeKind)-1;
}

/**
 * @brief Look up node metadata by name.
 *
 * Searches the topology's node hash map for a TopologyNodeInfo
 * associated with the given name.
 *
 * @param topology Topology instance.
 * @param name Node name to look up.
 * @return Pointer to the TopologyNodeInfo on success, or NULL if the
 *         node was not found or arguments are invalid.
 */
TopologyNodeInfo* topology_get_node_info(const Topology* topology, const char* name) {
  if (topology == NULL || name == NULL) {
    return NULL;
  }

  return hashmap_get(topology->nodes, name);
}

/**
 * @brief Look up a node object by name.
 *
 * Convenience wrapper around topology_get_node_info that extracts
 * the raw Node pointer.
 *
 * @param topology Topology instance.
 * @param name Node name to look up.
 * @return Pointer to the Node on success, or NULL if the node was not
 *         found.
 */
Node* topology_get_node(const Topology* topology, const char* name) {
  TopologyNodeInfo* info = topology_get_node_info(topology, name);
  return info != NULL ? info->node : NULL;
}

/**
 * @brief Count nodes of a specific kind in the topology.
 *
 * Iterates over all registered nodes and tallies those matching the
 * requested kind.
 *
 * @param topology Topology instance.
 * @param kind Node category to count.
 * @return Number of matching nodes, or 0 if the topology is NULL or
 *         has no nodes.
 */
size_t topology_count_nodes_of_kind(const Topology* topology, TopologyNodeKind kind) {
  if (topology == NULL || topology->nodes == NULL) {
    return 0U;
  }

  CountKindState state = {.kind = kind, .count = 0U};
  hashmap_foreach(topology->nodes, count_kind_cb, &state);
  return state.count;
}

/**
 * @brief Count the number of links in the topology.
 *
 * @param topology Topology instance.
 * @return Number of registered links, or 0 if the topology is NULL.
 */
size_t topology_count_links(const Topology* topology) {
  return topology != NULL && topology->links != NULL ? topology->links->count : 0U;
}

/**
 * @brief Register a new node in the topology.
 *
 * Creates a node through the registered node_ops callback, wraps it in a
 * TopologyNodeInfo, and inserts it into the topology's node map. The name
 * must be unique.
 *
 * @param topology Mutable topology.
 * @param kind Node category (host, switch, or router).
 * @param name Unique node name.
 * @return Pointer to the new TopologyNodeInfo on success, or NULL on
 *         failure (bad args, duplicate name, allocation failure).
 */
TopologyNodeInfo* topology_add_node(Topology* topology, TopologyNodeKind kind, const char* name) {
  if (topology == NULL || name == NULL || kind < TOPOLOGY_NODE_HOST ||
      kind > TOPOLOGY_NODE_ROUTER) {
    magi_errno = MAGI_ERR_BADARGS;
    return NULL;
  }

  if (hashmap_get(topology->nodes, name) != NULL) {
    magi_errno = MAGI_ERR_BADARGS;
    return NULL;
  }

  if (topology->node_ops == NULL || topology->node_ops->create_node == NULL) {
    magi_errno = MAGI_ERR_BADARGS;
    return NULL;
  }

  Node* node = topology->node_ops->create_node(kind, name);
  if (node == NULL) {
    return NULL;
  }

  TopologyNodeInfo* info = calloc(1, sizeof(*info));
  if (info == NULL) {
    node_free(node);
    magi_errno = MAGI_ERR_NOMEM;
    return NULL;
  }

  info->kind = kind;
  info->node = node;
  info->num_ports = 0U;

  if (hashmap_set(topology->nodes, name, info) != MAGI_OK) {
    free(info);
    node_free(node);
    return NULL;
  }

  return info;
}

/**
 * @brief Configure a host's IP and default gateway.
 *
 * Stores the CIDR and gateway strings in the node's metadata, then
 * delegates to the registered configure_host callback.
 *
 * @param topology Mutable topology.
 * @param name Host node name.
 * @param ip_address Host CIDR address string.
 * @param default_gateway Default gateway IP string.
 * @return MAGI_OK on success, or an error code if the node is not a
 *         host or configuration fails.
 */
int topology_configure_host(Topology* topology, const char* name, const char* ip_address,
                            const char* default_gateway) {
  TopologyNodeInfo* info = topology_get_node_info(topology, name);
  if (info == NULL || info->kind != TOPOLOGY_NODE_HOST) {
    magi_errno = MAGI_ERR_BADARGS;
    return MAGI_ERR_BADARGS;
  }

  if (ip_address != NULL && ip_address != info->ip_address) {
    snprintf(info->ip_address, sizeof(info->ip_address), "%s", ip_address);
  }
  if (default_gateway != NULL && default_gateway != info->default_gateway) {
    snprintf(info->default_gateway, sizeof(info->default_gateway), "%s", default_gateway);
  }

  if (topology->node_ops == NULL || topology->node_ops->configure_host == NULL) {
    magi_errno = MAGI_ERR_BADARGS;
    return MAGI_ERR_BADARGS;
  }

  return topology->node_ops->configure_host(info->node, info->ip_address, info->default_gateway);
}

/**
 * @brief Set the declared port count for a switch.
 *
 * Updates the metadata port count and delegates to the registered
 * configure_switch_num_ports callback.
 *
 * @param topology Mutable topology.
 * @param name Switch node name.
 * @param num_ports Number of ports to declare.
 * @return MAGI_OK on success, or an error code if the node is not a
 *         switch or configuration fails.
 */
int topology_configure_switch_num_ports(Topology* topology, const char* name, uint16_t num_ports) {
  TopologyNodeInfo* info = topology_get_node_info(topology, name);
  if (info == NULL || info->kind != TOPOLOGY_NODE_SWITCH) {
    magi_errno = MAGI_ERR_BADARGS;
    return MAGI_ERR_BADARGS;
  }

  info->num_ports = num_ports;
  if (topology->node_ops == NULL || topology->node_ops->configure_switch_num_ports == NULL) {
    magi_errno = MAGI_ERR_BADARGS;
    return MAGI_ERR_BADARGS;
  }

  return topology->node_ops->configure_switch_num_ports(info->node, num_ports);
}

/**
 * @brief Configure a single switch port's VLAN mode.
 *
 * Expands the switch's port count if the given port exceeds the current
 * maximum, then delegates to the registered configure_switch_port callback.
 *
 * @param topology Mutable topology.
 * @param name Switch node name.
 * @param port Port number to configure.
 * @param mode_text Port mode: "access" or "trunk".
 * @param vlan_id Access VLAN identifier.
 * @return MAGI_OK on success, or an error code if the node is not a
 *         switch or configuration fails.
 */
int topology_configure_switch_port(Topology* topology, const char* name, uint16_t port,
                                   const char* mode_text, uint16_t vlan_id) {
  TopologyNodeInfo* info = topology_get_node_info(topology, name);
  if (info == NULL || info->kind != TOPOLOGY_NODE_SWITCH) {
    magi_errno = MAGI_ERR_BADARGS;
    return MAGI_ERR_BADARGS;
  }

  if (port > info->num_ports) {
    info->num_ports = port;
  }

  if (topology->node_ops == NULL || topology->node_ops->configure_switch_port == NULL) {
    magi_errno = MAGI_ERR_BADARGS;
    return MAGI_ERR_BADARGS;
  }

  return topology->node_ops->configure_switch_port(info->node, port, mode_text, vlan_id);
}

/**
 * @brief Query an explicit switch port VLAN configuration.
 *
 * Delegates to the registered get_switch_port_config callback to retrieve
 * the mode and VLAN id for a given switch port.
 *
 * @param topology Topology instance.
 * @param name Switch node name.
 * @param port Port number to query.
 * @param mode_out Destination buffer for the mode text (may be NULL).
 * @param mode_len Size of the mode_out buffer.
 * @param vlan_id_out Destination for the VLAN id (may be NULL).
 * @return true if an explicit configuration exists for this port,
 *         false otherwise.
 */
bool topology_get_switch_port_config(const Topology* topology, const char* name, uint16_t port,
                                     char* mode_out, size_t mode_len, uint16_t* vlan_id_out) {
  TopologyNodeInfo* info = topology_get_node_info(topology, name);
  if (info == NULL || info->kind != TOPOLOGY_NODE_SWITCH) {
    return false;
  }

  if (topology->node_ops == NULL || topology->node_ops->get_switch_port_config == NULL) {
    return false;
  }

  return topology->node_ops->get_switch_port_config(info->node, port, mode_out, mode_len,
                                                    vlan_id_out);
}

/**
 * @brief Create a point-to-point link between two node ports.
 *
 * Adds interfaces to both nodes if they do not already exist, creates a
 * Link object, and registers it in the topology's link map. The link is
 * indexed by a normalized bidirectional key.
 *
 * @param topology Mutable topology.
 * @param node_a First endpoint node name.
 * @param port_a First endpoint port number (must be > 0).
 * @param node_b Second endpoint node name.
 * @param port_b Second endpoint port number (must be > 0).
 * @param delay_ms Link propagation delay in milliseconds.
 * @param mtu Link maximum transmission unit in bytes.
 * @return Pointer to the new TopologyLinkInfo on success, or NULL on
 *         failure (bad args, duplicate link, allocation failure).
 */
TopologyLinkInfo* topology_add_link(Topology* topology, const char* node_a, uint16_t port_a,
                                    const char* node_b, uint16_t port_b, uint32_t delay_ms,
                                    uint16_t mtu) {
  if (topology == NULL || node_a == NULL || node_b == NULL || port_a == 0U || port_b == 0U) {
    magi_errno = MAGI_ERR_BADARGS;
    return NULL;
  }

  TopologyNodeInfo* info_a = topology_get_node_info(topology, node_a);
  TopologyNodeInfo* info_b = topology_get_node_info(topology, node_b);

  if (info_a == NULL || info_b == NULL) {
    magi_errno = MAGI_ERR_BADARGS;
    return NULL;
  }

  Interface* iface_a = node_add_interface(info_a->node, port_a);
  Interface* iface_b = node_add_interface(info_b->node, port_b);

  if (iface_a == NULL || iface_b == NULL) {
    return NULL;
  }

  if ((iface_a->link != NULL && iface_a->link != iface_b->link) ||
      (iface_b->link != NULL && iface_b->link != iface_a->link)) {
    magi_errno = MAGI_ERR_BADARGS;
    return NULL;
  }

  char key[192];
  if (build_link_key(node_a, port_a, node_b, port_b, key, sizeof(key)) != MAGI_OK) {
    magi_errno = MAGI_ERR_BADARGS;
    return NULL;
  }

  if (hashmap_get(topology->links, key) != NULL) {
    magi_errno = MAGI_ERR_BADARGS;
    return NULL;
  }

  Link* link = link_new(iface_a, iface_b, delay_ms, mtu);
  if (link == NULL) {
    return NULL;
  }

  TopologyLinkInfo* info = calloc(1, sizeof(*info));
  if (info == NULL) {
    link_free(link);
    magi_errno = MAGI_ERR_NOMEM;
    return NULL;
  }

  info->link = link;
  snprintf(info->node_a, sizeof(info->node_a), "%s", node_a);
  info->port_a = port_a;
  snprintf(info->node_b, sizeof(info->node_b), "%s", node_b);
  info->port_b = port_b;

  if (hashmap_set(topology->links, key, info) != MAGI_OK) {
    link_free(link);
    free(info);
    return NULL;
  }

  return info;
}

/**
 * @brief Remove a point-to-point link between two node ports.
 *
 * Looks up the link by its normalized bidirectional key, frees the
 * underlying Link object and metadata, and removes the entry from the
 * topology's link map.
 *
 * @param topology Mutable topology.
 * @param node_a First endpoint node name.
 * @param port_a First endpoint port number.
 * @param node_b Second endpoint node name.
 * @param port_b Second endpoint port number.
 * @return MAGI_OK on success, or an error code if the link does not
 *         exist or arguments are invalid.
 */
int topology_remove_link(Topology* topology, const char* node_a, uint16_t port_a,
                         const char* node_b, uint16_t port_b) {
  if (topology == NULL || node_a == NULL || node_b == NULL) {
    magi_errno = MAGI_ERR_BADARGS;
    return MAGI_ERR_BADARGS;
  }

  char key[192];
  if (build_link_key(node_a, port_a, node_b, port_b, key, sizeof(key)) != MAGI_OK) {
    magi_errno = MAGI_ERR_BADARGS;
    return MAGI_ERR_BADARGS;
  }

  TopologyLinkInfo* info = hashmap_get(topology->links, key);
  if (info == NULL) {
    magi_errno = MAGI_ERR_BADARGS;
    return MAGI_ERR_BADARGS;
  }

  link_free(info->link);
  free(info);
  return hashmap_delete(topology->links, key);
}

/**
 * @brief Print one node category in sorted order.
 */
static void print_node_list(const Topology* topology, TopologyNodeKind kind) {
  if (topology == NULL || topology->nodes == NULL) {
    return;
  }

  size_t count = topology_count_nodes_of_kind(topology, kind);
  if (count == 0U) {
    LOG("TOPO", "%s (empty)", topology_kind_name(kind));
    return;
  }

  TopologyNodeInfo** items = calloc(count, sizeof(*items));
  if (items == NULL) {
    return;
  }

  NodeCollectState state = {.items = items, .kind = kind, .count = count, .index = 0U};
  hashmap_foreach(topology->nodes, collect_node_info, &state);
  qsort(items, state.index, sizeof(*items), compare_node_info);

  for (size_t index = 0; index < state.index; ++index) {
    TopologyNodeInfo* info = items[index];
    if (info != NULL) {
      LOG("TOPO", "%s %s", topology_kind_name(kind), info->node->name);
    }
  }

  free(items);
}

/**
 * @brief Print all links in sorted order.
 */
static void print_links(const Topology* topology) {
  if (topology == NULL || topology->links == NULL) {
    return;
  }

  size_t count = topology_count_links(topology);
  if (count == 0U) {
    LOG("TOPO", "LINKS (empty)");
    return;
  }

  TopologyLinkInfo** items = calloc(count, sizeof(*items));
  if (items == NULL) {
    return;
  }

  LinkCollectState state = {.items = items, .count = count, .index = 0U};
  hashmap_foreach(topology->links, collect_link_info, &state);
  qsort(items, count, sizeof(*items), compare_link_info);

  for (size_t index = 0; index < count; ++index) {
    TopologyLinkInfo* info = items[index];
    if (info != NULL) {
      LOG("TOPO", "%s:%u <-> %s:%u delay=%ums mtu=%u", info->node_a, (unsigned)info->port_a,
          info->node_b, (unsigned)info->port_b, (unsigned)info->link->delay_ms,
          (unsigned)info->link->mtu);
    }
  }

  free(items);
}

/**
 * @brief Print a human-readable summary of the topology.
 *
 * Displays all hosts, switches, routers, and links in sorted order
 * via the LOG macro.
 *
 * @param topology Topology instance to print. If NULL, prints
 *        "Topology is empty".
 */
void topology_print(const Topology* topology) {
  if (topology == NULL) {
    LOG("TOPO", "Topology is empty");
    return;
  }

  LOG("TOPO", "Hosts:");
  print_node_list(topology, TOPOLOGY_NODE_HOST);
  LOG("TOPO", "Switches:");
  print_node_list(topology, TOPOLOGY_NODE_SWITCH);
  LOG("TOPO", "Routers:");
  print_node_list(topology, TOPOLOGY_NODE_ROUTER);
  LOG("TOPO", "Links:");
  print_links(topology);
}

/**
 * @brief Replace one topology's state with another.
 *
 * Transfers ownership of the source topology's node map, link map, and
 * source path into the destination topology. The source topology is left
 * in an empty (NULL-ed) state so that topology_free may still be called
 * on it safely.
 *
 * @param destination Topology that will receive the source state.
 * @param source Topology that donates its state.
 * @return MAGI_OK on success, or MAGI_ERR_BADARGS if either argument is
 *         NULL.
 */
int topology_replace_contents(Topology* destination, Topology* source) {
  if (destination == NULL || source == NULL) {
    magi_errno = MAGI_ERR_BADARGS;
    return MAGI_ERR_BADARGS;
  }

  if (destination->links != NULL) {
    hashmap_foreach(destination->links, free_link_info, NULL);
    hashmap_free(destination->links);
  }

  if (destination->nodes != NULL) {
    hashmap_foreach(destination->nodes, free_node_info, NULL);
    hashmap_free(destination->nodes);
  }

  free(destination->source_path);

  destination->nodes = source->nodes;
  destination->links = source->links;
  destination->source_path = source->source_path;

  source->nodes = NULL;
  source->links = NULL;
  source->source_path = NULL;

  return MAGI_OK;
}
