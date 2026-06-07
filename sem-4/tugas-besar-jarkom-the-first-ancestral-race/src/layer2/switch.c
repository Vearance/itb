#define _POSIX_C_SOURCE 200809L

#include "switch.h"

#include "core/interface.h"
#include "layer2/ethernet.h"
#include "utils/arena.h"
#include "utils/log.h"
#include "utils/mac.h"
#include "utils/magi_error.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>

typedef struct SwitchMacEntry {
  uint16_t port;
  uint16_t vlan_id;
  char mac[18];
} SwitchMacEntry;

typedef struct SwitchState {
  HashMap* mac_table;
  HashMap* port_configs;
  uint16_t num_ports;
} SwitchState;

struct Switch {
  Node node;
};

typedef struct FloodCtx {
  Switch* sw;
  Interface* ingress;
  const EthernetFrame* frame;
  uint16_t vlan_id;
} FloodCtx;

typedef struct PrintMacCtx {
  const char* switch_name;
  size_t count;
} PrintMacCtx;

/**
 * Retrieve the SwitchState pointer from a Switch struct.
 *
 * Extracts the opaque data pointer from the embedded Node struct and casts
 * it to SwitchState. Returns NULL if the node pointer is NULL.
 *
 * \param sw Pointer to the Switch.
 * \return Pointer to the SwitchState, or NULL on failure.
 */
static SwitchState* switch_state(Switch* sw) {
  Node* node = switch_as_node(sw);
  return node != NULL ? (SwitchState*)node->data : NULL;
}

/**
 * Retrieve the const SwitchState pointer from a const Switch struct.
 *
 * Const-qualified version of switch_state(). Returns NULL if the node
 * pointer is NULL.
 *
 * \param sw Pointer to the const Switch.
 * \return Pointer to the const SwitchState, or NULL on failure.
 */
static const SwitchState* switch_state_const(const Switch* sw) {
  const Node* node = switch_as_node_const(sw);
  return node != NULL ? (const SwitchState*)node->data : NULL;
}

/**
 * Free a single hashmap entry whose value is a heap-allocated pointer.
 *
 * Callback for hashmap_foreach used to free MAC table and port config
 * value structs.
 *
 * \param key   Entry key (unused).
 * \param value Pointer to the heap-allocated struct to free.
 * \param ctx   User context (unused).
 */
static void free_value_entry(const char* key, void* value, void* ctx) {
  (void)key;
  (void)ctx;
  free(value);
}

/**
 * Free the SwitchState struct and all its internal resources.
 *
 * Iterates over and frees all MAC table entries and port configuration
 * entries, then frees the hash maps and the state struct itself.
 *
 * \param data Pointer to the SwitchState to free.
 */
static void switch_state_free(void* data) {
  SwitchState* state = data;
  if (state == NULL) {
    return;
  }

  hashmap_foreach(state->mac_table, free_value_entry, NULL);
  hashmap_free(state->mac_table);
  hashmap_foreach(state->port_configs, free_value_entry, NULL);
  hashmap_free(state->port_configs);
  free(state);
}

/**
 * Create and initialize a new SwitchState struct.
 *
 * Allocates a zero-initialized SwitchState and creates the MAC table and
 * port configuration hash maps, each with an initial capacity of 16 entries.
 *
 * \return Pointer to the new SwitchState, or NULL on allocation failure.
 */
static SwitchState* switch_state_new(void) {
  SwitchState* state = calloc(1U, sizeof(*state));
  if (state == NULL) {
    magi_errno = MAGI_ERR_NOMEM;
    return NULL;
  }

  state->mac_table = hashmap_new(16U);
  state->port_configs = hashmap_new(16U);
  if (state->mac_table == NULL || state->port_configs == NULL) {
    switch_state_free(state);
    magi_errno = MAGI_ERR_NOMEM;
    return NULL;
  }

  return state;
}

/**
 * Build a decimal string key for a port number.
 *
 * Writes the port number as an unsigned decimal string into the output
 * buffer for use as a hash map key.
 *
 * \param port Port number to format.
 * \param out  Output buffer (must be at least 16 bytes).
 */
static void build_port_key(uint16_t port, char out[16]) {
  snprintf(out, 16U, "%u", (unsigned)port);
}

/**
 * Build a compound hash map key from a VLAN ID and MAC address.
 *
 * Formats the key as "VLANID:AA:BB:CC:DD:EE:FF" for use in the MAC
 * address table.
 *
 * \param vlan_id VLAN identifier.
 * \param mac     6-byte MAC address.
 * \param out     Output buffer (must be at least 32 bytes).
 */
static void build_mac_key(uint16_t vlan_id, const uint8_t mac[ETHERNET_MAC_LEN], char out[32]) {
  char mac_text[18];
  mac_to_str(mac, mac_text);
  snprintf(out, 32U, "%u:%s", (unsigned)vlan_id, mac_text);
}

/**
 * Obtain the effective port configuration for a switch port.
 *
 * Returns the port mode (access or trunk) and associated VLAN ID. If no
 * explicit configuration is stored, defaults to access mode on VLAN 1.
 *
 * \param sw   Pointer to the Switch.
 * \param port Port number to query.
 * \param out  Pointer to a SwitchPortConfig to populate.
 */
static void switch_effective_port_config(const Switch* sw, uint16_t port, SwitchPortConfig* out) {
  if (out == NULL) {
    return;
  }

  out->mode = SWITCH_PORT_ACCESS;
  out->vlan_id = 1U;
  (void)switch_get_port_config(sw, port, out);
}

/**
 * Check whether a switch port allows traffic for a given VLAN.
 *
 * Trunk ports allow all VLANs. Access ports only allow their configured
 * VLAN ID.
 *
 * \param sw      Pointer to the Switch.
 * \param port    Port number to check.
 * \param vlan_id VLAN ID to test.
 * \return true if the port allows the VLAN, false otherwise.
 */
static bool switch_port_allows_vlan(const Switch* sw, uint16_t port, uint16_t vlan_id) {
  SwitchPortConfig config = {0};
  switch_effective_port_config(sw, port, &config);
  return config.mode == SWITCH_PORT_TRUNK || config.vlan_id == vlan_id;
}

/**
 * Learn the source MAC address from an incoming frame into the MAC table.
 *
 * Creates or updates a SwitchMacEntry keyed by VLAN ID and source MAC.
 * Broadcast and multicast source addresses are silently ignored.
 *
 * \param sw      Pointer to the Switch.
 * \param ingress Interface on which the frame arrived.
 * \param frame   Pointer to the parsed Ethernet frame.
 * \param vlan_id Resolved VLAN ID for the ingress port.
 * \return MAGI_OK on success, or a negative error code on failure.
 */
static int switch_learn_source(Switch* sw, Interface* ingress, const EthernetFrame* frame,
                               uint16_t vlan_id) {
  SwitchState* state = switch_state(sw);
  if (state == NULL || ingress == NULL || frame == NULL) {
    magi_errno = MAGI_ERR_BADARGS;
    return MAGI_ERR_BADARGS;
  }

  if (ethernet_mac_is_broadcast(frame->src_mac) || ethernet_mac_is_multicast(frame->src_mac)) {
    return MAGI_OK;
  }

  char key[32];
  build_mac_key(vlan_id, frame->src_mac, key);

  SwitchMacEntry* entry = hashmap_get(state->mac_table, key);
  if (entry == NULL) {
    entry = calloc(1U, sizeof(*entry));
    if (entry == NULL) {
      magi_errno = MAGI_ERR_NOMEM;
      return MAGI_ERR_NOMEM;
    }

    int status = hashmap_set(state->mac_table, key, entry);
    if (status != MAGI_OK) {
      free(entry);
      return status;
    }
  }

  entry->port = ingress->port_number;
  entry->vlan_id = vlan_id;
  mac_to_str(frame->src_mac, entry->mac);
  LOG(switch_as_node(sw)->name, "Learn MAC %s on VLAN %u Port %u", entry->mac, (unsigned)vlan_id,
      (unsigned)entry->port);
  return MAGI_OK;
}

/**
 * Send an Ethernet frame out through a specific switch port with proper VLAN handling.
 *
 * Checks whether the egress port allows the given VLAN. For trunk ports,
 * the 802.1Q VLAN tag is added to the outgoing frame. For access ports,
 * the tag is stripped (not present).
 *
 * \param sw       Pointer to the Switch.
 * \param egress   Egress interface to transmit through.
 * \param original Pointer to the original Ethernet frame to forward.
 * \param vlan_id  VLAN ID for VLAN tagging decisions.
 * \return MAGI_OK on success, or a negative error code on failure.
 */
static int switch_send_frame(Switch* sw, Interface* egress, const EthernetFrame* original,
                             uint16_t vlan_id) {
  if (sw == NULL || egress == NULL || original == NULL) {
    magi_errno = MAGI_ERR_BADARGS;
    return MAGI_ERR_BADARGS;
  }

  if (!switch_port_allows_vlan(sw, egress->port_number, vlan_id)) {
    return MAGI_OK;
  }

  SwitchPortConfig config = {0};
  switch_effective_port_config(sw, egress->port_number, &config);

  EthernetFrame out_frame = *original;
  out_frame.vlan_id = vlan_id;
  out_frame.vlan_present = config.mode == SWITCH_PORT_TRUNK;

  uint8_t* bytes = NULL;
  size_t len = 0U;
  int status = ethernet_frame_to_bytes(&out_frame, &bytes, &len);
  if (status != MAGI_OK) {
    return status;
  }

  LOG(switch_as_node(sw)->name, "Forward frame VLAN %u out Port %u", (unsigned)vlan_id,
      (unsigned)egress->port_number);
  status = interface_send(egress, bytes, len);
  return status;
}

/**
 * Callback for hashmap_foreach that floods a frame to all interfaces except the ingress.
 *
 * Skips interfaces that are NULL, match the ingress port, or have no link.
 * Delegates frame transmission to switch_send_frame.
 *
 * \param key   Interface port key (unused).
 * \param value Pointer to the Interface.
 * \param ctx   Pointer to a FloodCtx with flood parameters.
 */
static void flood_interface_cb(const char* key, void* value, void* ctx) {
  (void)key;

  FloodCtx* state = ctx;
  Interface* egress = value;
  if (state == NULL || egress == NULL || egress == state->ingress || egress->link == NULL) {
    return;
  }

  (void)switch_send_frame(state->sw, egress, state->frame, state->vlan_id);
}

/**
 * Flood a frame to all switch ports in the same VLAN except the ingress port.
 *
 * Iterates over all interfaces on the switch node and transmits the frame
 * to each eligible egress port via flood_interface_cb.
 *
 * \param sw      Pointer to the Switch.
 * \param ingress Interface on which the frame arrived (excluded from flood).
 * \param frame   Pointer to the Ethernet frame to flood.
 * \param vlan_id VLAN ID for the flood domain.
 * \return MAGI_OK on success, or a negative error code on failure.
 */
static int switch_flood(Switch* sw, Interface* ingress, const EthernetFrame* frame,
                        uint16_t vlan_id) {
  Node* node = switch_as_node(sw);
  if (node == NULL || node->interfaces == NULL || ingress == NULL || frame == NULL) {
    magi_errno = MAGI_ERR_BADARGS;
    return MAGI_ERR_BADARGS;
  }

  LOG(node->name, "Flood frame from Port %u on VLAN %u", (unsigned)ingress->port_number,
      (unsigned)vlan_id);
  FloodCtx ctx = {.sw = sw, .ingress = ingress, .frame = frame, .vlan_id = vlan_id};
  hashmap_foreach(node->interfaces, flood_interface_cb, &ctx);
  return MAGI_OK;
}

/**
 * Resolve the effective VLAN ID for a frame arriving on a given switch port.
 *
 * For access ports: if the frame carries a VLAN tag that does not match
 * the access VLAN, the frame is dropped. Otherwise the access VLAN ID is
 * used. For trunk ports: the frame's VLAN tag is used, defaulting to
 * VLAN 1 if untagged.
 *
 * \param sw       Pointer to the Switch.
 * \param ingress  Ingress interface.
 * \param frame    Pointer to the parsed Ethernet frame.
 * \param vlan_out Output pointer for the resolved VLAN ID.
 * \return true if the frame is accepted, false if it should be dropped.
 */
static bool switch_resolve_ingress_vlan(Switch* sw, Interface* ingress, const EthernetFrame* frame,
                                        uint16_t* vlan_out) {
  if (sw == NULL || ingress == NULL || frame == NULL || vlan_out == NULL) {
    return false;
  }

  SwitchPortConfig config = {0};
  switch_effective_port_config(sw, ingress->port_number, &config);

  if (config.mode == SWITCH_PORT_ACCESS) {
    if (frame->vlan_present && frame->vlan_id != config.vlan_id) {
      LOG(switch_as_node(sw)->name,
          "Drop tagged frame on access Port %u (tag VLAN %u, access VLAN %u)",
          (unsigned)ingress->port_number, (unsigned)frame->vlan_id, (unsigned)config.vlan_id);
      return false;
    }

    *vlan_out = config.vlan_id;
    return true;
  }

  *vlan_out = frame->vlan_present ? frame->vlan_id : 1U;
  return true;
}

/**
 * Handle an incoming Ethernet frame received on a switch interface.
 *
 * This is the top-level receive callback registered with the Node. The
 * pipeline is: parse Ethernet frame, resolve ingress VLAN, learn source
 * MAC, then flood or unicast forward based on the destination MAC address
 * lookup in the MAC table.
 *
 * \param node The Node (castable to Switch) that received the frame.
 * \param iface Interface on which the frame arrived.
 * \param data  Pointer to the raw frame bytes.
 * \param len   Length of the raw frame.
 */
static void switch_handle_receive(Node* node, Interface* iface, const uint8_t* data, size_t len) {
  Switch* sw = switch_from_node(node);
  SwitchState* state = switch_state(sw);
  arena_reset(node->arena);
  EthernetFrame frame = {0};

  if (state == NULL || iface == NULL || ethernet_frame_from_bytes(data, len, &frame) != MAGI_OK) {
    LOG(node->name, "Drop malformed Ethernet frame");
    return;
  }

  uint16_t vlan_id = 0U;
  if (!switch_resolve_ingress_vlan(sw, iface, &frame, &vlan_id)) {
    return;
  }

  (void)switch_learn_source(sw, iface, &frame, vlan_id);

  if (ethernet_mac_is_broadcast(frame.dst_mac) || ethernet_mac_is_multicast(frame.dst_mac)) {
    (void)switch_flood(sw, iface, &frame, vlan_id);
    return;
  }

  char key[32];
  build_mac_key(vlan_id, frame.dst_mac, key);
  SwitchMacEntry* entry = hashmap_get(state->mac_table, key);
  if (entry == NULL) {
    (void)switch_flood(sw, iface, &frame, vlan_id);
    return;
  }

  if (entry->port == iface->port_number) {
    LOG(node->name, "Destination is on ingress Port %u; drop loopback frame",
        (unsigned)iface->port_number);
    return;
  }

  Interface* egress = node_get_interface(node, entry->port);
  if (egress == NULL || egress->link == NULL) {
    LOG(node->name, "Known destination Port %u is unavailable; flood instead",
        (unsigned)entry->port);
    (void)switch_flood(sw, iface, &frame, vlan_id);
    return;
  }

  (void)switch_send_frame(sw, egress, &frame, vlan_id);
}

/**
 * Print a single MAC table entry via the logging system.
 *
 * Callback for hashmap_foreach used by switch_print_mac_table. Each entry
 * is logged as "MAC VLAN <id> <mac> -> Port <port>".
 *
 * \param key   MAC table key (unused).
 * \param value Pointer to a SwitchMacEntry.
 * \param ctx   Pointer to a PrintMacCtx tracking switch name and count.
 */
static void print_mac_entry(const char* key, void* value, void* ctx) {
  (void)key;

  PrintMacCtx* state = ctx;
  SwitchMacEntry* entry = value;
  if (state == NULL || entry == NULL) {
    return;
  }

  LOG(state->switch_name, "MAC VLAN %u %s -> Port %u", (unsigned)entry->vlan_id, entry->mac,
      (unsigned)entry->port);
  state->count++;
}

Switch* switch_new(const char* name) {
  Node* node = node_new(name);
  if (node == NULL) {
    return NULL;
  }

  SwitchState* state = switch_state_new();
  if (state == NULL) {
    node_free(node);
    return NULL;
  }

  node->data = state;
  node->data_free = switch_state_free;
  node->handle_receive = switch_handle_receive;
  return switch_from_node(node);
}

void switch_free(Switch* sw) {
  node_free(switch_as_node(sw));
}

Node* switch_as_node(Switch* sw) {
  return (Node*)sw;
}

const Node* switch_as_node_const(const Switch* sw) {
  return (const Node*)sw;
}

Switch* switch_from_node(Node* node) {
  return (Switch*)node;
}

const Switch* switch_from_node_const(const Node* node) {
  return (const Switch*)node;
}

int switch_configure_num_ports(Switch* sw, uint16_t num_ports) {
  SwitchState* state = switch_state(sw);
  if (sw == NULL || state == NULL) {
    magi_errno = MAGI_ERR_BADARGS;
    return MAGI_ERR_BADARGS;
  }

  state->num_ports = num_ports;
  for (uint16_t port = 1U; port <= num_ports; ++port) {
    if (node_add_interface(switch_as_node(sw), port) == NULL) {
      return MAGI_ERR_BADARGS;
    }
  }

  return MAGI_OK;
}

int switch_configure_port(Switch* sw, uint16_t port, const char* mode_text, uint16_t vlan_id) {
  SwitchState* state = switch_state(sw);
  if (sw == NULL || state == NULL || port == 0U || mode_text == NULL) {
    magi_errno = MAGI_ERR_BADARGS;
    return MAGI_ERR_BADARGS;
  }

  SwitchPortConfig parsed = {0};
  if (strcasecmp(mode_text, "access") == 0) {
    parsed.mode = SWITCH_PORT_ACCESS;
    parsed.vlan_id = vlan_id != ETHERNET_VLAN_ID_NONE ? vlan_id : 1U;
  } else if (strcasecmp(mode_text, "trunk") == 0) {
    parsed.mode = SWITCH_PORT_TRUNK;
    parsed.vlan_id = ETHERNET_VLAN_ID_NONE;
  } else {
    magi_errno = MAGI_ERR_BADARGS;
    return MAGI_ERR_BADARGS;
  }

  if (node_add_interface(switch_as_node(sw), port) == NULL) {
    return MAGI_ERR_BADARGS;
  }

  char key[16];
  build_port_key(port, key);
  SwitchPortConfig* config = hashmap_get(state->port_configs, key);
  if (config == NULL) {
    config = malloc(sizeof(*config));
    if (config == NULL) {
      magi_errno = MAGI_ERR_NOMEM;
      return MAGI_ERR_NOMEM;
    }

    int status = hashmap_set(state->port_configs, key, config);
    if (status != MAGI_OK) {
      free(config);
      return status;
    }
  }

  *config = parsed;
  if (port > state->num_ports) {
    state->num_ports = port;
  }

  return MAGI_OK;
}

bool switch_get_port_config(const Switch* sw, uint16_t port, SwitchPortConfig* out) {
  const SwitchState* state = switch_state_const(sw);
  if (state == NULL || port == 0U || out == NULL) {
    return false;
  }

  char key[16];
  build_port_key(port, key);
  SwitchPortConfig* config = hashmap_get(state->port_configs, key);
  if (config == NULL) {
    return false;
  }

  *out = *config;
  return true;
}

void switch_print_mac_table(const Switch* sw) {
  const SwitchState* state = switch_state_const(sw);
  if (sw == NULL || state == NULL) {
    LOG("SWITCH", "MAC table unavailable");
    return;
  }

  const Node* node = switch_as_node_const(sw);
  PrintMacCtx ctx = {.switch_name = node->name, .count = 0U};
  hashmap_foreach(state->mac_table, print_mac_entry, &ctx);
  if (ctx.count == 0U) {
    LOG(node->name, "MAC table empty");
  }
}
