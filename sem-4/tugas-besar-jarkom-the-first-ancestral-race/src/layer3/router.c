#define _POSIX_C_SOURCE 200809L

#include "router.h"

#include "core/interface.h"
#include "layer3/icmp.h"
#include "layer3/ipv4.h"
#include "utils/arena.h"
#include "utils/byteops.h"
#include "utils/log.h"
#include "utils/mac.h"
#include "utils/magi_error.h"

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define ROUTER_ETHERNET_MAC_LEN 6U
#define ROUTER_ETHERNET_MIN_LEN 14U
#define ROUTER_ETHERNET_VLAN_LEN 18U
#define ROUTER_ETHERTYPE_IPV4 0x0800U
#define ROUTER_ETHERTYPE_ARP 0x0806U
#define ROUTER_ETHERTYPE_VLAN 0x8100U
#define ROUTER_ARP_LEN 28U
#define ROUTER_ARP_REQUEST 1U
#define ROUTER_ARP_REPLY 2U

struct Router {
  Node node;
};

typedef struct RouterFrame {
  uint8_t dst_mac[ROUTER_ETHERNET_MAC_LEN];
  uint8_t src_mac[ROUTER_ETHERNET_MAC_LEN];
  uint16_t ethertype;
  bool vlan_present;
  uint16_t vlan_id;
  const uint8_t* payload;
  size_t payload_len;
} RouterFrame;

typedef struct RouterArpMessage {
  uint16_t opcode;
  uint8_t sender_mac[ROUTER_ETHERNET_MAC_LEN];
  uint8_t sender_ip[4];
  uint8_t target_mac[ROUTER_ETHERNET_MAC_LEN];
  uint8_t target_ip[4];
} RouterArpMessage;

typedef struct RouterPendingPacket {
  uint8_t* payload;
  size_t payload_len;
  uint16_t out_port;
  uint16_t vlan_id;
  struct RouterPendingPacket* next;
} RouterPendingPacket;

typedef struct RouterState {
  RoutingTableEntry* routes;
  size_t route_count;
  size_t route_cap;
  HashMap* arp_cache;
  HashMap* pending;
  RoutingTableEntry scratch_route;
  uint16_t next_id;
} RouterState;

Node* router_as_node(Router* router) {
  return router != NULL ? &router->node : NULL;
}

const Node* router_as_node_const(const Router* router) {
  return router != NULL ? &router->node : NULL;
}

Router* router_from_node(Node* node) {
  return (Router*)node;
}

const Router* router_from_node_const(const Node* node) {
  return (const Router*)node;
}

/**
 * @brief Get the mutable RouterState from a Router.
 *
 * @param router The router instance.
 * @return Pointer to RouterState, or NULL.
 */
static RouterState* router_state(Router* router) {
  Node* node = router_as_node(router);
  return node != NULL ? (RouterState*)node->data : NULL;
}

/**
 * @brief Get the const RouterState from a Router.
 *
 * @param router The router instance.
 * @return Pointer to const RouterState, or NULL.
 */
static const RouterState* router_state_const(const Router* router) {
  const Node* node = router_as_node_const(router);
  return node != NULL ? (const RouterState*)node->data : NULL;
}

/**
 * @brief Get the router's name from the embedded Node.
 *
 * Falls back to "ROUTER" if the node is NULL.
 *
 * @param router The router instance.
 * @return The node name string.
 */
static const char* router_name(const Router* router) {
  const Node* node = router_as_node_const(router);
  return node != NULL ? node->name : "ROUTER";
}

/**
 * @brief Check whether a MAC address is the broadcast address (FF:FF:FF:FF:FF:FF).
 *
 * @param mac 6-byte MAC address.
 * @return true if all bytes are 0xFF.
 */
static bool mac_is_broadcast(const uint8_t mac[ROUTER_ETHERNET_MAC_LEN]) {
  if (mac == NULL) {
    return false;
  }

  for (size_t index = 0U; index < ROUTER_ETHERNET_MAC_LEN; ++index) {
    if (mac[index] != 0xFFU) {
      return false;
    }
  }

  return true;
}

/**
 * @brief Compare two MAC addresses for equality.
 *
 * @param lhs First MAC address.
 * @param rhs Second MAC address.
 * @return true if both are non-NULL and identical.
 */
static bool mac_equal(const uint8_t lhs[ROUTER_ETHERNET_MAC_LEN],
                      const uint8_t rhs[ROUTER_ETHERNET_MAC_LEN]) {
  return lhs != NULL && rhs != NULL && memcmp(lhs, rhs, ROUTER_ETHERNET_MAC_LEN) == 0;
}

/**
 * @brief Fill a 6-byte buffer with the Ethernet broadcast address (FF:FF:FF:FF:FF:FF).
 *
 * @param out Output buffer for the broadcast MAC.
 */
static void mac_broadcast(uint8_t out[ROUTER_ETHERNET_MAC_LEN]) {
  if (out != NULL) {
    memset(out, 0xFF, ROUTER_ETHERNET_MAC_LEN);
  }
}

/**
 * @brief Duplicate a string on the heap.
 *
 * @param text The null-terminated string to copy.
 * @return Heap-allocated copy, or NULL on allocation failure.
 */
static char* duplicate_string(const char* text) {
  if (text == NULL) {
    return NULL;
  }

  size_t len = strlen(text) + 1U;
  char* copy = malloc(len);
  if (copy == NULL) {
    magi_errno = MAGI_ERR_NOMEM;
    return NULL;
  }

  memcpy(copy, text, len);
  return copy;
}

/**
 * @brief hashmap_foreach callback to free heap-allocated string values.
 *
 * @param key   The hashmap entry key (unused).
 * @param value The string value to free.
 * @param ctx   User context (unused).
 */
static void free_string_entry(const char* key, void* value, void* ctx) {
  (void)key;
  (void)ctx;
  free(value);
}

/**
 * @brief Free a linked list of RouterPendingPacket structs.
 *
 * Each packet's payload buffer and the packet itself are freed.
 *
 * @param packet The head of the pending packet list.
 */
static void free_pending_list(RouterPendingPacket* packet) {
  while (packet != NULL) {
    RouterPendingPacket* next = packet->next;
    free(packet->payload);
    free(packet);
    packet = next;
  }
}

/**
 * @brief hashmap_foreach callback to free pending packet lists.
 *
 * @param key   The hashmap entry key (unused).
 * @param value The RouterPendingPacket list head.
 * @param ctx   User context (unused).
 */
static void free_pending_entry(const char* key, void* value, void* ctx) {
  (void)key;
  (void)ctx;
  free_pending_list(value);
}

/**
 * @brief Free a RouterState and all its owned resources.
 *
 * Frees the routing table, ARP cache entries, pending packet queues,
 * and the state struct itself. Compatible with node->data_free.
 *
 * @param data The RouterState pointer to free.
 */
static void router_state_free(void* data) {
  RouterState* state = data;
  if (state == NULL) {
    return;
  }

  free(state->routes);
  hashmap_foreach(state->arp_cache, free_string_entry, NULL);
  hashmap_free(state->arp_cache);
  hashmap_foreach(state->pending, free_pending_entry, NULL);
  hashmap_free(state->pending);
  free(state);
}

/**
 * @brief Allocate and initialise a new RouterState.
 *
 * Creates ARP cache and pending hash maps, sets next_id to 1.
 *
 * @return Pointer to the new RouterState, or NULL on allocation failure.
 */
static RouterState* router_state_new(void) {
  RouterState* state = calloc(1U, sizeof(*state));
  if (state == NULL) {
    magi_errno = MAGI_ERR_NOMEM;
    return NULL;
  }

  state->arp_cache = hashmap_new(16U);
  state->pending = hashmap_new(16U);
  state->next_id = 1U;
  if (state->arp_cache == NULL || state->pending == NULL) {
    router_state_free(state);
    magi_errno = MAGI_ERR_NOMEM;
    return NULL;
  }

  return state;
}

/**
 * @brief Parse raw Ethernet frame bytes into a RouterFrame struct.
 *
 * Handles both untagged (14-byte header) and 802.1Q VLAN-tagged
 * (18-byte header) frames. Sets the payload pointer inside the
 * input buffer.
 *
 * @param data      Raw frame bytes.
 * @param len       Length of the data.
 * @param frame_out Output RouterFrame.
 * @return MAGI_OK on success, or an error code.
 */
static int parse_frame(const uint8_t* data, size_t len, RouterFrame* frame_out) {
  if (data == NULL || frame_out == NULL || len < ROUTER_ETHERNET_MIN_LEN) {
    magi_errno = MAGI_ERR_BADARGS;
    return MAGI_ERR_BADARGS;
  }

  memset(frame_out, 0, sizeof(*frame_out));
  memcpy(frame_out->dst_mac, data, ROUTER_ETHERNET_MAC_LEN);
  memcpy(frame_out->src_mac, data + ROUTER_ETHERNET_MAC_LEN, ROUTER_ETHERNET_MAC_LEN);

  uint16_t ethertype = READ_U16(data, 12U);
  if (ethertype == ROUTER_ETHERTYPE_VLAN) {
    if (len < ROUTER_ETHERNET_VLAN_LEN) {
      magi_errno = MAGI_ERR_BADARGS;
      return MAGI_ERR_BADARGS;
    }

    frame_out->vlan_present = true;
    frame_out->vlan_id = (uint16_t)(READ_U16(data, 14U) & 0x0FFFU);
    frame_out->ethertype = READ_U16(data, 16U);
    frame_out->payload = data + ROUTER_ETHERNET_VLAN_LEN;
    frame_out->payload_len = len - ROUTER_ETHERNET_VLAN_LEN;
    return MAGI_OK;
  }

  frame_out->ethertype = ethertype;
  frame_out->payload = data + ROUTER_ETHERNET_MIN_LEN;
  frame_out->payload_len = len - ROUTER_ETHERNET_MIN_LEN;
  return MAGI_OK;
}

/**
 * @brief Serialize a RouterFrame to a heap-allocated byte buffer.
 *
 * Handles both untagged and VLAN-tagged header formats.
 *
 * @param frame    The frame to serialise.
 * @param bytes_out Output pointer for the allocated byte buffer.
 * @param len_out   Output length.
 * @return MAGI_OK on success, or an error code.
 */
static int frame_to_bytes(const RouterFrame* frame, uint8_t** bytes_out, size_t* len_out) {
  if (frame == NULL || bytes_out == NULL || len_out == NULL ||
      (frame->payload_len > 0U && frame->payload == NULL)) {
    magi_errno = MAGI_ERR_BADARGS;
    return MAGI_ERR_BADARGS;
  }

  size_t header_len = frame->vlan_present ? ROUTER_ETHERNET_VLAN_LEN : ROUTER_ETHERNET_MIN_LEN;
  size_t total_len = header_len + frame->payload_len;
  uint8_t* bytes = malloc(total_len);
  if (bytes == NULL) {
    magi_errno = MAGI_ERR_NOMEM;
    return MAGI_ERR_NOMEM;
  }

  memcpy(bytes, frame->dst_mac, ROUTER_ETHERNET_MAC_LEN);
  memcpy(bytes + ROUTER_ETHERNET_MAC_LEN, frame->src_mac, ROUTER_ETHERNET_MAC_LEN);
  if (frame->vlan_present) {
    WRITE_U16(bytes, 12U, ROUTER_ETHERTYPE_VLAN);
    WRITE_U16(bytes, 14U, frame->vlan_id & 0x0FFFU);
    WRITE_U16(bytes, 16U, frame->ethertype);
  } else {
    WRITE_U16(bytes, 12U, frame->ethertype);
  }

  if (frame->payload_len > 0U) {
    memcpy(bytes + header_len, frame->payload, frame->payload_len);
  }

  *bytes_out = bytes;
  *len_out = total_len;
  return MAGI_OK;
}

/**
 * @brief Parse raw ARP message bytes into a RouterArpMessage struct.
 *
 * Validates hardware type (Ethernet), protocol type (IPv4), and
 * address lengths before extracting fields.
 *
 * @param data         Raw ARP message bytes (28 bytes).
 * @param len          Length of the data.
 * @param message_out  Output RouterArpMessage.
 * @return MAGI_OK on success, or an error code.
 */
static int arp_from_bytes(const uint8_t* data, size_t len, RouterArpMessage* message_out) {
  if (data == NULL || message_out == NULL || len < ROUTER_ARP_LEN) {
    magi_errno = MAGI_ERR_BADARGS;
    return MAGI_ERR_BADARGS;
  }

  if (READ_U16(data, 0U) != 1U || READ_U16(data, 2U) != ROUTER_ETHERTYPE_IPV4 ||
      data[4] != ROUTER_ETHERNET_MAC_LEN || data[5] != 4U) {
    magi_errno = MAGI_ERR_BADARGS;
    return MAGI_ERR_BADARGS;
  }

  memset(message_out, 0, sizeof(*message_out));
  message_out->opcode = READ_U16(data, 6U);
  memcpy(message_out->sender_mac, data + 8U, ROUTER_ETHERNET_MAC_LEN);
  memcpy(message_out->sender_ip, data + 14U, 4U);
  memcpy(message_out->target_mac, data + 18U, ROUTER_ETHERNET_MAC_LEN);
  memcpy(message_out->target_ip, data + 24U, 4U);
  return MAGI_OK;
}

/**
 * @brief Serialize a RouterArpMessage to a heap-allocated byte buffer.
 *
 * Produces the standard 28-byte ARP message format (Ethernet/IPv4).
 *
 * @param message   The ARP message to serialise.
 * @param bytes_out Output pointer for the allocated byte buffer.
 * @param len_out   Output length (always 28).
 * @return MAGI_OK on success, or an error code.
 */
static int arp_to_bytes(const RouterArpMessage* message, uint8_t** bytes_out, size_t* len_out) {
  if (message == NULL || bytes_out == NULL || len_out == NULL) {
    magi_errno = MAGI_ERR_BADARGS;
    return MAGI_ERR_BADARGS;
  }

  uint8_t* bytes = calloc(ROUTER_ARP_LEN, 1U);
  if (bytes == NULL) {
    magi_errno = MAGI_ERR_NOMEM;
    return MAGI_ERR_NOMEM;
  }

  WRITE_U16(bytes, 0U, 1U);
  WRITE_U16(bytes, 2U, ROUTER_ETHERTYPE_IPV4);
  WRITE_U8(bytes, 4U, ROUTER_ETHERNET_MAC_LEN);
  WRITE_U8(bytes, 5U, 4U);
  WRITE_U16(bytes, 6U, message->opcode);
  memcpy(bytes + 8U, message->sender_mac, ROUTER_ETHERNET_MAC_LEN);
  memcpy(bytes + 14U, message->sender_ip, 4U);
  memcpy(bytes + 18U, message->target_mac, ROUTER_ETHERNET_MAC_LEN);
  memcpy(bytes + 24U, message->target_ip, 4U);

  *bytes_out = bytes;
  *len_out = ROUTER_ARP_LEN;
  return MAGI_OK;
}

/**
 * @brief Check whether a frame's VLAN tag is allowed on an interface.
 *
 * If the interface has no VLAN configured (vlan_id == 0), all frames
 * are allowed. Otherwise, untagged frames are accepted and tagged
 * frames must match the interface VLAN.
 *
 * @param iface The ingress interface.
 * @param frame The received Ethernet frame.
 * @return true if the frame is allowed.
 */
static bool frame_vlan_allowed(const Interface* iface, const RouterFrame* frame) {
  if (iface == NULL || frame == NULL || iface->vlan_id == 0U) {
    return true;
  }

  return !frame->vlan_present || frame->vlan_id == iface->vlan_id;
}

/**
 * @brief Determine the egress VLAN ID to use when sending a frame.
 *
 * If the egress interface has a VLAN configured, that VLAN is used.
 * Otherwise, the ingress frame's VLAN is carried through (trunk behaviour).
 *
 * @param iface   The egress interface.
 * @param ingress The ingress Ethernet frame (may be NULL).
 * @return The VLAN ID to write into the outgoing frame.
 */
static uint16_t egress_vlan_id(const Interface* iface, const RouterFrame* ingress) {
  if (iface != NULL && iface->vlan_id != 0U) {
    return iface->vlan_id;
  }

  return ingress != NULL && ingress->vlan_present ? ingress->vlan_id : 0U;
}

/**
 * @brief Build and send an Ethernet frame from a router.
 *
 * Constructs a RouterFrame with the given parameters, serialises it,
 * and sends it via interface_send. The frame is heap-allocated and
 * freed by the link layer after transmission.
 *
 * @param router      The router instance.
 * @param iface       The egress interface.
 * @param dst_mac     Destination MAC address.
 * @param ethertype   EtherType field value.
 * @param payload     Payload bytes.
 * @param payload_len Length of the payload.
 * @param vlan_id     VLAN ID (0 = untagged).
 * @return MAGI_OK on success, or an error code.
 */
static int router_send_ethernet(Router* router, Interface* iface, const uint8_t dst_mac[6],
                                uint16_t ethertype, const uint8_t* payload, size_t payload_len,
                                uint16_t vlan_id) {
  if (router == NULL || iface == NULL || dst_mac == NULL || (payload_len > 0U && payload == NULL)) {
    magi_errno = MAGI_ERR_BADARGS;
    return MAGI_ERR_BADARGS;
  }

  RouterFrame frame = {0};
  memcpy(frame.dst_mac, dst_mac, ROUTER_ETHERNET_MAC_LEN);
  memcpy(frame.src_mac, iface->mac, ROUTER_ETHERNET_MAC_LEN);
  frame.ethertype = ethertype;
  frame.vlan_present = vlan_id != 0U;
  frame.vlan_id = vlan_id;
  frame.payload = payload;
  frame.payload_len = payload_len;

  uint8_t* bytes = NULL;
  size_t len = 0U;
  int status = frame_to_bytes(&frame, &bytes, &len);
  if (status != MAGI_OK) {
    return status;
  }

  char dst_text[18];
  mac_to_str(dst_mac, dst_text);
  LOG(router_name(router), "Send Ethernet frame port=%u vlan=%u dst=%s ethertype=0x%04X",
      (unsigned)iface->port_number, (unsigned)vlan_id, dst_text, (unsigned)ethertype);
  return interface_send(iface, bytes, len);
}

/**
 * @brief Insert or update an entry in the router's ARP cache.
 *
 * If an entry already exists for the IP, it is overwritten. The MAC is
 * stored as its colon-separated string representation.
 *
 * @param router The router instance.
 * @param ip     The IPv4 address.
 * @param mac    The associated MAC address.
 * @return MAGI_OK on success, or an error code.
 */
static int router_cache_arp(Router* router, const uint8_t ip[4], const uint8_t mac[6]) {
  RouterState* state = router_state(router);
  if (state == NULL || ip == NULL || mac == NULL) {
    magi_errno = MAGI_ERR_BADARGS;
    return MAGI_ERR_BADARGS;
  }

  char ip_text[16];
  char mac_text[18];
  ipv4_address_to_string(ip, ip_text);
  mac_to_str(mac, mac_text);

  char* existing = hashmap_get(state->arp_cache, ip_text);
  if (existing != NULL) {
    snprintf(existing, 18U, "%s", mac_text);
    return MAGI_OK;
  }

  char* stored = duplicate_string(mac_text);
  if (stored == NULL) {
    return MAGI_ERR_NOMEM;
  }

  int status = hashmap_set(state->arp_cache, ip_text, stored);
  if (status != MAGI_OK) {
    free(stored);
    return status;
  }

  return MAGI_OK;
}

/**
 * @brief Look up a MAC address in the router's ARP cache.
 *
 * @param router  The router instance.
 * @param ip      The IPv4 address to look up.
 * @param mac_out Output buffer for the 6-byte MAC address.
 * @return true if the entry was found and mac_out was populated.
 */
static bool router_lookup_arp(Router* router, const uint8_t ip[4], uint8_t mac_out[6]) {
  RouterState* state = router_state(router);
  if (state == NULL || ip == NULL || mac_out == NULL) {
    return false;
  }

  char ip_text[16];
  ipv4_address_to_string(ip, ip_text);
  char* mac_text = hashmap_get(state->arp_cache, ip_text);
  return mac_text != NULL && mac_from_str(mac_text, mac_out) == MAGI_OK;
}

/**
 * @brief Queue a packet while the ARP resolution for a next hop is pending.
 *
 * Allocates a RouterPendingPacket, copies the payload, and appends it
 * to the linked list for the given next-hop IP.
 *
 * @param router      The router instance.
 * @param next_hop    The next-hop IPv4 address being resolved.
 * @param out_port    The egress port number.
 * @param vlan_id     The VLAN ID to use when sending queued packets.
 * @param payload     The packet payload (copied).
 * @param payload_len Length of the payload.
 * @return MAGI_OK on success, or an error code.
 */
static int queue_pending_packet(Router* router, const uint8_t next_hop[4], uint16_t out_port,
                                uint16_t vlan_id, const uint8_t* payload, size_t payload_len) {
  RouterState* state = router_state(router);
  if (state == NULL || next_hop == NULL || (payload_len > 0U && payload == NULL)) {
    magi_errno = MAGI_ERR_BADARGS;
    return MAGI_ERR_BADARGS;
  }

  RouterPendingPacket* packet = calloc(1U, sizeof(*packet));
  if (packet == NULL) {
    magi_errno = MAGI_ERR_NOMEM;
    return MAGI_ERR_NOMEM;
  }

  if (payload_len > 0U) {
    packet->payload = malloc(payload_len);
    if (packet->payload == NULL) {
      free(packet);
      magi_errno = MAGI_ERR_NOMEM;
      return MAGI_ERR_NOMEM;
    }
    memcpy(packet->payload, payload, payload_len);
  }

  packet->payload_len = payload_len;
  packet->out_port = out_port;
  packet->vlan_id = vlan_id;

  char next_hop_text[16];
  ipv4_address_to_string(next_hop, next_hop_text);
  RouterPendingPacket* head = hashmap_get(state->pending, next_hop_text);
  if (head == NULL) {
    int status = hashmap_set(state->pending, next_hop_text, packet);
    if (status != MAGI_OK) {
      free_pending_list(packet);
      return status;
    }
  } else {
    RouterPendingPacket* tail = head;
    while (tail->next != NULL) {
      tail = tail->next;
    }
    tail->next = packet;
  }

  LOG(router_name(router), "Queue IPv4 packet while ARP resolves %s", next_hop_text);
  return MAGI_OK;
}

/**
 * @brief Flush all queued packets for a resolved next hop.
 *
 * Retrieves the pending packet list for the given IP, removes it from
 * the hashmap, and sends each packet via router_send_ethernet using
 * the now-known destination MAC.
 *
 * @param router   The router instance.
 * @param next_hop The resolved next-hop IPv4 address.
 * @param dst_mac  The resolved MAC address.
 * @return MAGI_OK on success, or the last error encountered.
 */
static int flush_pending_packets(Router* router, const uint8_t next_hop[4],
                                 const uint8_t dst_mac[6]) {
  RouterState* state = router_state(router);
  if (state == NULL || next_hop == NULL || dst_mac == NULL) {
    magi_errno = MAGI_ERR_BADARGS;
    return MAGI_ERR_BADARGS;
  }

  char next_hop_text[16];
  ipv4_address_to_string(next_hop, next_hop_text);
  RouterPendingPacket* packet = hashmap_get(state->pending, next_hop_text);
  if (packet == NULL) {
    return MAGI_OK;
  }

  (void)hashmap_delete(state->pending, next_hop_text);
  LOG(router_name(router), "Send queued IPv4 packet(s) for %s", next_hop_text);

  int final_status = MAGI_OK;
  while (packet != NULL) {
    RouterPendingPacket* next = packet->next;
    Interface* iface = node_get_interface(router_as_node(router), packet->out_port);
    if (iface != NULL) {
      int status = router_send_ethernet(router, iface, dst_mac, ROUTER_ETHERTYPE_IPV4,
                                        packet->payload, packet->payload_len, packet->vlan_id);
      if (status != MAGI_OK) {
        final_status = status;
      }
    }

    free(packet->payload);
    free(packet);
    packet = next;
  }

  return final_status;
}

/**
 * @brief Extract the IPv4 address from an interface's ip_address string.
 *
 * @param iface  The interface whose IP to extract.
 * @param ip_out Output buffer for the 4-byte IPv4 address.
 * @return MAGI_OK on success, or an error code.
 */
static int interface_ip(const Interface* iface, uint8_t ip_out[4]) {
  if (iface == NULL || iface->ip_address[0] == '\0') {
    magi_errno = MAGI_ERR_BADARGS;
    return MAGI_ERR_BADARGS;
  }

  return ipv4_parse_address(iface->ip_address, ip_out);
}

/**
 * @brief Check whether an interface has a specific IPv4 address.
 *
 * @param iface The interface to check.
 * @param ip    The IPv4 address to compare against.
 * @return true if the interface's IP matches.
 */
static bool router_interface_matches_ip(const Interface* iface, const uint8_t ip[4]) {
  uint8_t iface_ip[4];
  return iface != NULL && ip != NULL && interface_ip(iface, iface_ip) == MAGI_OK &&
         ipv4_addr_equal(iface_ip, ip);
}

/**
 * @brief Find an interface on the router by its IPv4 address.
 *
 * Iterates the node's interface hashmap and returns the first interface
 * whose configured IP matches.
 *
 * @param router The router instance.
 * @param ip     The IPv4 address to look for.
 * @return Pointer to the matching Interface, or NULL.
 */
static Interface* router_find_interface_by_ip(Router* router, const uint8_t ip[4]) {
  Node* node = router_as_node(router);
  if (node == NULL || node->interfaces == NULL || ip == NULL) {
    return NULL;
  }

  for (size_t index = 0U; index < node->interfaces->capacity; ++index) {
    HashEntry* entry = &node->interfaces->entries[index];
    if (entry->key != NULL && !entry->tombstone && router_interface_matches_ip(entry->value, ip)) {
      return entry->value;
    }
  }

  return NULL;
}

/**
 * @brief Build and send an ARP request for a target IP.
 *
 * Constructs an ARP request message, wraps it in an Ethernet broadcast
 * frame, and sends it on the specified interface.
 *
 * @param router    The router instance.
 * @param iface     The egress interface.
 * @param target_ip The IPv4 address to resolve.
 * @param vlan_id   VLAN ID for the outgoing frame (0 = untagged).
 * @return MAGI_OK on success, or an error code.
 */
static int router_send_arp_request(Router* router, Interface* iface, const uint8_t target_ip[4],
                                   uint16_t vlan_id) {
  if (router == NULL || iface == NULL || target_ip == NULL) {
    magi_errno = MAGI_ERR_BADARGS;
    return MAGI_ERR_BADARGS;
  }

  RouterArpMessage request = {0};
  request.opcode = ROUTER_ARP_REQUEST;
  memcpy(request.sender_mac, iface->mac, ROUTER_ETHERNET_MAC_LEN);
  if (interface_ip(iface, request.sender_ip) != MAGI_OK) {
    return MAGI_ERR_BADARGS;
  }
  memcpy(request.target_ip, target_ip, 4U);

  uint8_t* arp_bytes = NULL;
  size_t arp_len = 0U;
  int status = arp_to_bytes(&request, &arp_bytes, &arp_len);
  if (status != MAGI_OK) {
    return status;
  }

  uint8_t broadcast[ROUTER_ETHERNET_MAC_LEN];
  char target_text[16];
  mac_broadcast(broadcast);
  ipv4_address_to_string(target_ip, target_text);
  LOG(router_name(router), "ARP Miss for %s. Send ARP Request", target_text);
  status = router_send_ethernet(router, iface, broadcast, ROUTER_ETHERTYPE_ARP, arp_bytes, arp_len,
                                vlan_id);
  free(arp_bytes);
  return status;
}

/**
 * @brief qsort-compatible comparator for routing table entries.
 *
 * Sorts entries in descending prefix length order so that more specific
 * routes appear first (for display and iteration convenience).
 *
 * @param lhs Left entry.
 * @param rhs Right entry.
 * @return Negative if lhs is more specific, positive if rhs is more specific.
 */
static int compare_routes_desc(const void* lhs, const void* rhs) {
  const RoutingTableEntry* left = lhs;
  const RoutingTableEntry* right = rhs;
  return right->prefix_len - left->prefix_len;
}

/**
 * @brief Ensure the routing table array has capacity for at least `needed` entries.
 *
 * Grows the array by doubling if necessary.
 *
 * @param state  The router state.
 * @param needed The minimum required capacity.
 * @return MAGI_OK on success, or MAGI_ERR_NOMEM.
 */
static int ensure_route_capacity(RouterState* state, size_t needed) {
  if (state == NULL) {
    magi_errno = MAGI_ERR_BADARGS;
    return MAGI_ERR_BADARGS;
  }

  if (state->route_cap >= needed) {
    return MAGI_OK;
  }

  size_t new_cap = state->route_cap == 0U ? 8U : state->route_cap * 2U;
  while (new_cap < needed) {
    new_cap *= 2U;
  }

  RoutingTableEntry* routes = realloc(state->routes, new_cap * sizeof(*routes));
  if (routes == NULL) {
    magi_errno = MAGI_ERR_NOMEM;
    return MAGI_ERR_NOMEM;
  }

  state->routes = routes;
  state->route_cap = new_cap;
  return MAGI_OK;
}

/**
 * @brief Check whether a routing table entry matches a destination IP.
 *
 * Uses ipv4_addr_in_network to test (dst_ip & mask) == network.
 *
 * @param route  The routing table entry to test.
 * @param dst_ip The destination IPv4 address.
 * @return true if the route covers the destination.
 */
static bool route_matches(const RoutingTableEntry* route, const uint8_t dst_ip[4]) {
  return route != NULL && dst_ip != NULL &&
         ipv4_addr_in_network(dst_ip, route->network, route->mask);
}

/**
 * @brief Check whether a routing table entry matches a given network/prefix.
 *
 * Used to detect duplicate routes and for route removal.
 *
 * @param route      The routing table entry.
 * @param network    The network address to compare.
 * @param prefix_len The prefix length to compare.
 * @return true if both network and prefix_len match.
 */
static bool static_route_same_dest(const RoutingTableEntry* route, const uint8_t network[4],
                                   int prefix_len) {
  return route != NULL && network != NULL && route->prefix_len == prefix_len &&
         ipv4_addr_equal(route->network, network);
}

/**
 * @brief Derive a directly connected routing table entry from an interface.
 *
 * Parses the interface's CIDR address and populates the RoutingTableEntry
 * with the network, mask, prefix length, port, and metric 1.
 *
 * @param iface The interface to derive from.
 * @param out   Output RoutingTableEntry.
 * @return true if the interface has a valid IP and the entry was populated.
 */
static bool interface_connected_route(const Interface* iface, RoutingTableEntry* out) {
  if (iface == NULL || out == NULL || iface->ip_address[0] == '\0') {
    return false;
  }

  memset(out, 0, sizeof(*out));
  uint8_t iface_ip[4];
  if (ipv4_parse_cidr(iface->ip_address, iface_ip, out->network, out->mask, &out->prefix_len) !=
      MAGI_OK) {
    return false;
  }

  (void)iface_ip;
  out->out_port = iface->port_number;
  out->metric = 1U;
  return true;
}

/**
 * @brief Look up a route and send an IPv4 packet from a router.
 *
 * Performs an LPM lookup for the destination IP. If found, serialises
 * the packet and sends it as an Ethernet frame via the egress interface.
 * If the next-hop MAC is not in the ARP cache, the packet is queued
 * and an ARP request is sent.
 *
 * @param router The router instance.
 * @param pkt    The IPv4 packet to forward.
 * @return MAGI_OK on success, or an error code.
 */
static int router_send_ipv4_packet(Router* router, IPv4Packet* pkt) {
  RouterState* state = router_state(router);
  if (router == NULL || pkt == NULL || state == NULL) {
    magi_errno = MAGI_ERR_BADARGS;
    return MAGI_ERR_BADARGS;
  }

  const RoutingTableEntry* route = lpm_lookup(router, pkt->dst_ip);
  if (route == NULL) {
    magi_errno = MAGI_ERR_NOROUTE;
    return MAGI_ERR_NOROUTE;
  }

  Interface* egress = node_get_interface(router_as_node(router), route->out_port);
  if (egress == NULL || egress->link == NULL) {
    magi_errno = MAGI_ERR_NOLINK;
    return MAGI_ERR_NOLINK;
  }

  uint8_t* bytes = NULL;
  size_t bytes_len = 0U;
  int status = ipv4_packet_to_bytes(pkt, &bytes, &bytes_len);
  if (status != MAGI_OK) {
    return status;
  }

  uint8_t next_hop[4];
  memcpy(next_hop, ipv4_addr_is_zero(route->next_hop) ? pkt->dst_ip : route->next_hop, 4U);

  uint8_t dst_mac[ROUTER_ETHERNET_MAC_LEN];
  uint16_t vlan_id = egress->vlan_id;
  if (router_lookup_arp(router, next_hop, dst_mac)) {
    status = router_send_ethernet(router, egress, dst_mac, ROUTER_ETHERTYPE_IPV4, bytes, bytes_len,
                                  vlan_id);
    free(bytes);
    return status;
  }

  status = queue_pending_packet(router, next_hop, egress->port_number, vlan_id, bytes, bytes_len);
  if (status == MAGI_OK) {
    status = router_send_arp_request(router, egress, next_hop, vlan_id);
  }
  free(bytes);
  return status;
}

/**
 * @brief Build and send an ICMP error message (Time Exceeded or Destination Unreachable).
 *
 * Constructs an ICMP error with the original packet's IP header and
 * first 8 bytes of transport header embedded in the payload. Suppresses
 * errors triggered by ICMP error messages to avoid error loops.
 *
 * @param router       The router instance.
 * @param original_pkt The original IPv4 packet that caused the error.
 * @param original_raw The raw bytes of the original IP header.
 * @param type         ICMP type (Time Exceeded or Destination Unreachable).
 * @param code         ICMP code.
 * @return MAGI_OK on success, or an error code.
 */
static int router_send_icmp_error(Router* router, const IPv4Packet* original_pkt,
                                  const uint8_t* original_raw, uint8_t type, uint8_t code) {
  if (router == NULL || original_pkt == NULL || original_raw == NULL) {
    magi_errno = MAGI_ERR_BADARGS;
    return MAGI_ERR_BADARGS;
  }

  if (original_pkt->protocol == IPV4_PROTOCOL_ICMP && original_pkt->payload_len > 0U &&
      (original_pkt->payload[0] == ICMP_TYPE_DEST_UNREACHABLE ||
       original_pkt->payload[0] == ICMP_TYPE_TIME_EXCEEDED)) {
    return MAGI_OK;
  }

  const RoutingTableEntry* route = lpm_lookup(router, original_pkt->src_ip);
  if (route == NULL) {
    return MAGI_ERR_NOROUTE;
  }

  Interface* egress = node_get_interface(router_as_node(router), route->out_port);
  if (egress == NULL) {
    return MAGI_ERR_BADARGS;
  }

  uint8_t src_ip[4];
  if (interface_ip(egress, src_ip) != MAGI_OK) {
    return MAGI_ERR_BADARGS;
  }

  size_t embedded_len = original_pkt->total_len < 28U ? original_pkt->total_len : 28U;
  size_t error_payload_len = 4U + embedded_len;
  uint8_t* error_payload = calloc(error_payload_len, 1U);
  if (error_payload == NULL) {
    magi_errno = MAGI_ERR_NOMEM;
    return MAGI_ERR_NOMEM;
  }
  memcpy(error_payload + 4U, original_raw, embedded_len);

  ICMPMessage icmp = {0};
  icmp.type = type;
  icmp.code = code;
  icmp.payload = error_payload;
  icmp.payload_len = error_payload_len;

  uint8_t* icmp_bytes = NULL;
  size_t icmp_len = 0U;
  int status = icmp_message_to_bytes(&icmp, &icmp_bytes, &icmp_len);
  free(error_payload);
  if (status != MAGI_OK) {
    return status;
  }

  IPv4Packet reply = {0};
  reply.version_ihl = IPV4_VERSION_IHL;
  reply.identification = router_state(router)->next_id++;
  reply.ttl = IPV4_DEFAULT_TTL;
  reply.protocol = IPV4_PROTOCOL_ICMP;
  memcpy(reply.src_ip, src_ip, 4U);
  memcpy(reply.dst_ip, original_pkt->src_ip, 4U);
  reply.payload = icmp_bytes;
  reply.payload_len = icmp_len;

  char dst_text[16];
  ipv4_address_to_string(original_pkt->src_ip, dst_text);
  LOG(router_name(router), "Send ICMP type=%u code=%u to %s", (unsigned)type, (unsigned)code,
      dst_text);
  status = router_send_ipv4_packet(router, &reply);
  free(icmp_bytes);
  return status;
}

/**
 * @brief Build and send an ICMP Echo Reply in response to a ping.
 *
 * Constructs an ICMP Echo Reply from the incoming Echo Request, swaps
 * source and destination IPs, and sends via the ingress interface's
 * subnet.
 *
 * @param router  The router instance.
 * @param ingress The interface on which the Echo Request arrived.
 * @param request The IPv4 packet containing the Echo Request.
 * @param echo    The parsed ICMP Echo Request message.
 * @return MAGI_OK on success, or an error code.
 */
static int router_send_echo_reply(Router* router, Interface* ingress, const IPv4Packet* request,
                                  const ICMPMessage* echo) {
  if (router == NULL || ingress == NULL || request == NULL || echo == NULL) {
    magi_errno = MAGI_ERR_BADARGS;
    return MAGI_ERR_BADARGS;
  }

  uint8_t src_ip[4];
  if (interface_ip(ingress, src_ip) != MAGI_OK) {
    return MAGI_ERR_BADARGS;
  }

  ICMPMessage reply_icmp = {0};
  reply_icmp.type = ICMP_TYPE_ECHO_REPLY;
  reply_icmp.code = 0U;
  reply_icmp.identifier = echo->identifier;
  reply_icmp.sequence = echo->sequence;
  reply_icmp.payload = echo->payload;
  reply_icmp.payload_len = echo->payload_len;

  uint8_t* icmp_bytes = NULL;
  size_t icmp_len = 0U;
  int status = icmp_message_to_bytes(&reply_icmp, &icmp_bytes, &icmp_len);
  if (status != MAGI_OK) {
    return status;
  }

  IPv4Packet reply = {0};
  reply.version_ihl = IPV4_VERSION_IHL;
  reply.identification = router_state(router)->next_id++;
  reply.ttl = IPV4_DEFAULT_TTL;
  reply.protocol = IPV4_PROTOCOL_ICMP;
  memcpy(reply.src_ip, src_ip, 4U);
  memcpy(reply.dst_ip, request->src_ip, 4U);
  reply.payload = icmp_bytes;
  reply.payload_len = icmp_len;

  status = router_send_ipv4_packet(router, &reply);
  free(icmp_bytes);
  return status;
}

/**
 * @brief Check whether an IPv4 packet is addressed to the router itself.
 *
 * If the destination IP matches one of the router's interfaces and the
 * protocol is ICMP, it handles the message (currently only Echo Reply).
 * Always returns true if the packet is destined for the router, so that
 * the caller does not attempt to forward it.
 *
 * @param router The router instance.
 * @param iface  The ingress interface (may be NULL).
 * @param pkt    The received IPv4 packet.
 * @return true if the packet was destined for this router.
 */
static bool router_handle_local_ipv4(Router* router, Interface* iface, const IPv4Packet* pkt) {
  Interface* matching_iface = router_find_interface_by_ip(router, pkt->dst_ip);
  if (matching_iface == NULL) {
    return false;
  }

  if (pkt->protocol != IPV4_PROTOCOL_ICMP) {
    return true;
  }

  ICMPMessage msg = {0};
  if (icmp_unpack(&msg, pkt->payload, pkt->payload_len) != MAGI_OK) {
    return true;
  }

  if (msg.type == ICMP_TYPE_ECHO_REQUEST) {
    char src_text[16];
    ipv4_address_to_string(pkt->src_ip, src_text);
    LOG(router_name(router), "ICMP echo request for router interface from %s", src_text);
    (void)router_send_echo_reply(router, iface != NULL ? iface : matching_iface, pkt, &msg);
  }

  return true;
}

/**
 * @brief Handle an incoming ARP message (request or reply).
 *
 * Parses the ARP payload from the Ethernet frame, records the sender's
 * IP-MAC mapping in the cache, and responds if the ARP request is for
 * one of this router's interfaces.
 *
 * @param router The router instance.
 * @param iface  The ingress interface.
 * @param frame  The parsed Ethernet frame containing the ARP message.
 */
static void router_handle_arp(Router* router, Interface* iface, const RouterFrame* frame) {
  RouterArpMessage message = {0};
  if (router == NULL || iface == NULL || frame == NULL ||
      arp_from_bytes(frame->payload, frame->payload_len, &message) != MAGI_OK) {
    LOG(router_name(router), "Drop malformed ARP payload");
    return;
  }

  (void)router_cache_arp(router, message.sender_ip, message.sender_mac);

  char sender_ip[16];
  char target_ip[16];
  char sender_mac[18];
  ipv4_address_to_string(message.sender_ip, sender_ip);
  ipv4_address_to_string(message.target_ip, target_ip);
  mac_to_str(message.sender_mac, sender_mac);

  if (message.opcode == ROUTER_ARP_REPLY) {
    LOG(router_name(router), "ARP Reply received: %s is at %s", sender_ip, sender_mac);
    (void)flush_pending_packets(router, message.sender_ip, message.sender_mac);
    return;
  }

  if (message.opcode != ROUTER_ARP_REQUEST ||
      !router_interface_matches_ip(iface, message.target_ip)) {
    return;
  }

  RouterArpMessage reply = {0};
  reply.opcode = ROUTER_ARP_REPLY;
  memcpy(reply.sender_mac, iface->mac, ROUTER_ETHERNET_MAC_LEN);
  memcpy(reply.target_mac, message.sender_mac, ROUTER_ETHERNET_MAC_LEN);
  memcpy(reply.sender_ip, message.target_ip, 4U);
  memcpy(reply.target_ip, message.sender_ip, 4U);

  uint8_t* arp_bytes = NULL;
  size_t arp_len = 0U;
  if (arp_to_bytes(&reply, &arp_bytes, &arp_len) != MAGI_OK) {
    return;
  }

  LOG(router_name(router), "Send ARP Reply to %s (%s)", sender_ip, sender_mac);
  (void)router_send_ethernet(router, iface, message.sender_mac, ROUTER_ETHERTYPE_ARP, arp_bytes,
                             arp_len, egress_vlan_id(iface, frame));
  free(arp_bytes);
}

/**
 * @brief Forward an IPv4 packet not destined for the router.
 *
 * Decrements TTL (if TTL <=1, sends ICMP Time Exceeded), performs an
 * LPM lookup, and sends the packet via router_send_ipv4_packet. If no
 * route is found, sends ICMP Destination Unreachable.
 *
 * @param router       The router instance.
 * @param original_raw The raw IP header bytes (for embedding in ICMP errors).
 * @param pkt          The received IPv4 packet to forward.
 */
static void router_forward_ipv4(Router* router, const uint8_t* original_raw,
                                const IPv4Packet* pkt) {
  if (router == NULL || original_raw == NULL || pkt == NULL) {
    return;
  }

  if (pkt->ttl <= 1U) {
    char src_text[16];
    ipv4_address_to_string(pkt->src_ip, src_text);
    LOG(router_name(router), "TTL expired while forwarding packet from %s", src_text);
    (void)router_send_icmp_error(router, pkt, original_raw, ICMP_TYPE_TIME_EXCEEDED, 0U);
    return;
  }

  const RoutingTableEntry* route = lpm_lookup(router, pkt->dst_ip);
  if (route == NULL) {
    char dst_text[16];
    ipv4_address_to_string(pkt->dst_ip, dst_text);
    LOG(router_name(router), "No route to %s", dst_text);
    (void)router_send_icmp_error(router, pkt, original_raw, ICMP_TYPE_DEST_UNREACHABLE, 0U);
    return;
  }

  IPv4Packet forward = *pkt;
  forward.ttl = (uint8_t)(pkt->ttl - 1U);
  char dst_text[16];
  ipv4_address_to_string(pkt->dst_ip, dst_text);
  LOG(router_name(router), "Forward IPv4 dst=%s ttl=%u out_port=%u", dst_text,
      (unsigned)forward.ttl, (unsigned)route->out_port);
  (void)router_send_ipv4_packet(router, &forward);
}

void router_handle_receive(Node* node, Interface* in_iface, const uint8_t* data, size_t len) {
  Router* router = router_from_node(node);
  arena_reset(node->arena);
  RouterFrame frame = {0};
  if (router == NULL || in_iface == NULL || data == NULL ||
      parse_frame(data, len, &frame) != MAGI_OK) {
    LOG(node != NULL ? node->name : "ROUTER", "Drop malformed Ethernet frame");
    return;
  }

  if (!frame_vlan_allowed(in_iface, &frame)) {
    LOG(router_name(router), "Drop frame on Port %u: VLAN %u is not allowed",
        (unsigned)in_iface->port_number, (unsigned)frame.vlan_id);
    return;
  }

  if (frame.ethertype == ROUTER_ETHERTYPE_ARP) {
    router_handle_arp(router, in_iface, &frame);
    return;
  }

  if (frame.ethertype != ROUTER_ETHERTYPE_IPV4) {
    return;
  }

  if (!mac_equal(frame.dst_mac, in_iface->mac) && !mac_is_broadcast(frame.dst_mac)) {
    return;
  }

  IPv4Packet pkt = {0};
  if (ipv4_unpack(&pkt, frame.payload, frame.payload_len) != MAGI_OK) {
    LOG(router_name(router), "Drop IPv4 packet: bad header/checksum");
    return;
  }

  if (router_handle_local_ipv4(router, in_iface, &pkt)) {
    return;
  }

  router_forward_ipv4(router, frame.payload, &pkt);
}

Router* router_new(const char* name) {
  Node* node = node_new(name);
  if (node == NULL) {
    return NULL;
  }

  RouterState* state = router_state_new();
  if (state == NULL) {
    node_free(node);
    return NULL;
  }

  node->data = state;
  node->data_free = router_state_free;
  node->handle_receive = router_handle_receive;
  return router_from_node(node);
}

void router_free(Router* router) {
  node_free(router_as_node(router));
}

int router_add_route(Router* router, const char* dest_cidr, const char* next_hop_ip,
                     uint16_t out_port) {
  RouterState* state = router_state(router);
  if (state == NULL || dest_cidr == NULL || out_port == 0U) {
    magi_errno = MAGI_ERR_BADARGS;
    return MAGI_ERR_BADARGS;
  }

  RoutingTableEntry route = {0};
  uint8_t parsed_ip[4];
  if (ipv4_parse_cidr(dest_cidr, parsed_ip, route.network, route.mask, &route.prefix_len) !=
      MAGI_OK) {
    return MAGI_ERR_BADARGS;
  }
  (void)parsed_ip;

  if (next_hop_ip != NULL && next_hop_ip[0] != '\0' && strcmp(next_hop_ip, "direct") != 0) {
    if (ipv4_parse_address(next_hop_ip, route.next_hop) != MAGI_OK) {
      return MAGI_ERR_BADARGS;
    }
  }

  route.out_port = out_port;
  route.metric = 1U;

  for (size_t index = 0U; index < state->route_count; ++index) {
    if (static_route_same_dest(&state->routes[index], route.network, route.prefix_len)) {
      state->routes[index] = route;
      qsort(state->routes, state->route_count, sizeof(*state->routes), compare_routes_desc);
      return MAGI_OK;
    }
  }

  int status = ensure_route_capacity(state, state->route_count + 1U);
  if (status != MAGI_OK) {
    return status;
  }

  state->routes[state->route_count++] = route;
  qsort(state->routes, state->route_count, sizeof(*state->routes), compare_routes_desc);
  return MAGI_OK;
}

int router_remove_route(Router* router, const char* dest_cidr) {
  RouterState* state = router_state(router);
  if (state == NULL || dest_cidr == NULL) {
    magi_errno = MAGI_ERR_BADARGS;
    return MAGI_ERR_BADARGS;
  }

  RoutingTableEntry parsed = {0};
  uint8_t ip[4];
  if (ipv4_parse_cidr(dest_cidr, ip, parsed.network, parsed.mask, &parsed.prefix_len) != MAGI_OK) {
    return MAGI_ERR_BADARGS;
  }

  for (size_t index = 0U; index < state->route_count; ++index) {
    if (static_route_same_dest(&state->routes[index], parsed.network, parsed.prefix_len)) {
      if (index + 1U < state->route_count) {
        memmove(&state->routes[index], &state->routes[index + 1U],
                (state->route_count - index - 1U) * sizeof(*state->routes));
      }
      state->route_count--;
      return MAGI_OK;
    }
  }

  magi_errno = MAGI_ERR_NOROUTE;
  return MAGI_ERR_NOROUTE;
}

const RoutingTableEntry* lpm_lookup(Router* router, const uint8_t dst_ip[4]) {
  RouterState* state = router_state(router);
  if (state == NULL || dst_ip == NULL) {
    return NULL;
  }

  const RoutingTableEntry* best = NULL;
  int best_prefix = -1;
  for (size_t index = 0U; index < state->route_count; ++index) {
    const RoutingTableEntry* route = &state->routes[index];
    if (route->prefix_len > best_prefix && route_matches(route, dst_ip)) {
      best = route;
      best_prefix = route->prefix_len;
    }
  }

  Node* node = router_as_node(router);
  if (node != NULL && node->interfaces != NULL) {
    for (size_t index = 0U; index < node->interfaces->capacity; ++index) {
      HashEntry* entry = &node->interfaces->entries[index];
      if (entry->key == NULL || entry->tombstone) {
        continue;
      }

      RoutingTableEntry connected = {0};
      if (interface_connected_route(entry->value, &connected) &&
          connected.prefix_len > best_prefix && route_matches(&connected, dst_ip)) {
        state->scratch_route = connected;
        best = &state->scratch_route;
        best_prefix = connected.prefix_len;
      }
    }
  }

  return best;
}

/**
 * @brief hashmap_foreach callback to print a single ARP cache entry.
 *
 * Logs the IP-to-MAC mapping using the node name from ctx.
 *
 * @param key   The IP address string.
 * @param value The MAC address string.
 * @param ctx   The node name string (for LOG output).
 */
static void router_print_arp_entry(const char* key, void* value, void* ctx) {
  const char* node_name = (const char*)ctx;
  if (node_name == NULL) {
    return;
  }
  const char* mac = (const char*)value;
  LOG(node_name, "ARP %s -> %s", key, mac != NULL ? mac : "(null)");
}

void router_print_arp_cache(const Router* router) {
  const RouterState* state = router_state_const(router);
  if (router == NULL || state == NULL) {
    LOG("ROUTER", "ARP cache unavailable");
    return;
  }

  const Node* node = router_as_node_const(router);
  hashmap_foreach(state->arp_cache, router_print_arp_entry, (void*)node->name);
  if (state->arp_cache == NULL || state->arp_cache->count == 0U) {
    LOG(router_name(router), "ARP cache empty");
  }
}

void router_foreach_route(const Router* router, router_route_visitor_fn fn, void* ctx) {
  const RouterState* state = router_state_const(router);
  if (state == NULL || fn == NULL) {
    return;
  }

  for (size_t index = 0U; index < state->route_count; ++index) {
    fn(&state->routes[index], ctx);
  }
}

void router_print_routes(const Router* router) {
  const RouterState* state = router_state_const(router);
  if (router == NULL || state == NULL) {
    LOG("ROUTER", "Routing table unavailable");
    return;
  }

  const Node* node = router_as_node_const(router);
  size_t count = 0U;
  for (size_t index = 0U; index < state->route_count; ++index) {
    char dest[32];
    char next_hop[16];
    (void)ipv4_format_cidr(state->routes[index].network, state->routes[index].prefix_len, dest,
                           sizeof(dest));
    ipv4_address_to_string(state->routes[index].next_hop, next_hop);
    LOG(router_name(router), "Route S %s via %s port %u", dest,
        ipv4_addr_is_zero(state->routes[index].next_hop) ? "direct" : next_hop,
        (unsigned)state->routes[index].out_port);
    count++;
  }

  if (node != NULL && node->interfaces != NULL) {
    for (size_t index = 0U; index < node->interfaces->capacity; ++index) {
      const HashEntry* entry = &node->interfaces->entries[index];
      if (entry == NULL || entry->key == NULL || entry->tombstone) {
        continue;
      }

      RoutingTableEntry connected = {0};
      if (!interface_connected_route(entry->value, &connected)) {
        continue;
      }

      char dest[32];
      (void)ipv4_format_cidr(connected.network, connected.prefix_len, dest, sizeof(dest));
      LOG(router_name(router), "Route C %s direct port %u", dest, (unsigned)connected.out_port);
      count++;
    }
  }

  if (count == 0U) {
    LOG(router_name(router), "Routing table empty");
  }
}
