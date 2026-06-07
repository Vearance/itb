#define _POSIX_C_SOURCE 200809L

#include "host.h"

#include "core/interface.h"
#include "layer2/arp.h"
#include "layer2/ethernet.h"
#include "utils/log.h"
#include "utils/mac.h"
#include "utils/magi_error.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct PendingPacket {
  uint8_t* payload;
  size_t payload_len;
  uint16_t ethertype;
  uint16_t port;
  struct PendingPacket* next;
} PendingPacket;

typedef struct HostState {
  char ip_address[64];
  char ip_key[16];
  char default_gateway[64];
  HashMap* arp_cache;
  HashMap* pending;
} HostState;

struct Host {
  Node node;
};

typedef struct PrintArpCtx {
  const char* host_name;
  size_t count;
} PrintArpCtx;

/**
 * Retrieve the HostState pointer from a Host struct.
 *
 * Extracts the opaque data pointer from the embedded Node struct and casts
 * it to HostState. Returns NULL if the node pointer is NULL.
 *
 * @param host Pointer to the Host.
 * @return Pointer to the HostState, or NULL on failure.
 */
static HostState* host_state(Host* host) {
  Node* node = host_as_node(host);
  return node != NULL ? (HostState*)node->data : NULL;
}

/**
 * Retrieve the const HostState pointer from a const Host struct.
 *
 * Const-qualified version of host_state(). Returns NULL if the node
 * pointer is NULL.
 *
 * @param host Pointer to the const Host.
 * @return Pointer to the const HostState, or NULL on failure.
 */
static const HostState* host_state_const(const Host* host) {
  const Node* node = host_as_node_const(host);
  return node != NULL ? (const HostState*)node->data : NULL;
}

/**
 * Free a single hashmap entry whose value is a heap-allocated string.
 *
 * Callback for hashmap_foreach used to free ARP cache string values.
 *
 * @param key   Entry key (unused).
 * @param value Pointer to the heap-allocated string to free.
 * @param ctx   User context (unused).
 */
static void free_string_entry(const char* key, void* value, void* ctx) {
  (void)key;
  (void)ctx;
  free(value);
}

/**
 * Free a linked list of PendingPacket structs.
 *
 * Iterates through the list, freeing each packet's payload buffer and
 * the packet struct itself.
 *
 * @param packet Head of the pending packet list to free.
 */
static void free_pending_list(PendingPacket* packet) {
  while (packet != NULL) {
    PendingPacket* next = packet->next;
    free(packet->payload);
    free(packet);
    packet = next;
  }
}

/**
 * Free a single hashmap entry whose value is a pending packet list.
 *
 * Callback for hashmap_foreach used to free pending queue entries.
 *
 * @param key   Entry key (unused).
 * @param value Pointer to the PendingPacket list head to free.
 * @param ctx   User context (unused).
 */
static void free_pending_entry(const char* key, void* value, void* ctx) {
  (void)key;
  (void)ctx;
  free_pending_list((PendingPacket*)value);
}

/**
 * Free the HostState struct and all its internal resources.
 *
 * Iterates over and frees all ARP cache entries and all pending packet
 * queues, then frees the hash maps and the state struct itself.
 *
 * @param data Pointer to the HostState to free.
 */
static void host_state_free(void* data) {
  HostState* state = data;
  if (state == NULL) {
    return;
  }

  hashmap_foreach(state->arp_cache, free_string_entry, NULL);
  hashmap_free(state->arp_cache);
  hashmap_foreach(state->pending, free_pending_entry, NULL);
  hashmap_free(state->pending);
  free(state);
}

/**
 * Create and initialize a new HostState struct.
 *
 * Allocates a zero-initialized HostState and creates the ARP cache and
 * pending packet hash maps, each with an initial capacity of 8 entries.
 *
 * @return Pointer to the new HostState, or NULL on allocation failure.
 */
static HostState* host_state_new(void) {
  HostState* state = calloc(1U, sizeof(*state));
  if (state == NULL) {
    magi_errno = MAGI_ERR_NOMEM;
    return NULL;
  }

  state->arp_cache = hashmap_new(8U);
  state->pending = hashmap_new(8U);
  if (state->arp_cache == NULL || state->pending == NULL) {
    host_state_free(state);
    magi_errno = MAGI_ERR_NOMEM;
    return NULL;
  }

  return state;
}

/**
 * Duplicate a null-terminated string on the heap.
 *
 * Allocates and copies the string including the null terminator.
 *
 * @param text Null-terminated string to duplicate.
 * @return Heap-allocated copy of the string, or NULL on failure.
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
 * Normalize an IP address string to canonical dotted-decimal form.
 *
 * Parses the input (which may include a CIDR prefix) and writes only the
 * address portion into the output buffer in dotted-decimal format.
 *
 * @param ip_text Input IP string (e.g. "192.168.1.1/24").
 * @param out     Output buffer of at least 16 bytes.
 * @return MAGI_OK on success, or MAGI_ERR_BADARGS if parsing fails.
 */
static int normalize_ip_key(const char* ip_text, char out[16]) {
  uint8_t ip[4];
  int status = arp_ipv4_from_string(ip_text, ip);
  if (status != MAGI_OK) {
    return status;
  }

  arp_ipv4_to_string(ip, out);
  return MAGI_OK;
}

/**
 * Insert or update a MAC address in the host's ARP cache for a given IP.
 *
 * If an entry already exists for the IP, its MAC text is overwritten.
 * Otherwise, a new heap-allocated copy of the MAC string is stored.
 *
 * @param host Pointer to the Host.
 * @param ip   4-byte IPv4 address to use as the cache key.
 * @param mac  6-byte MAC address to cache.
 * @return MAGI_OK on success, or a negative error code on failure.
 */
static int host_cache_arp(Host* host, const uint8_t ip[4], const uint8_t mac[6]) {
  HostState* state = host_state(host);
  if (state == NULL || ip == NULL || mac == NULL) {
    magi_errno = MAGI_ERR_BADARGS;
    return MAGI_ERR_BADARGS;
  }

  char ip_key[16];
  char mac_text[18];
  arp_ipv4_to_string(ip, ip_key);
  mac_to_str(mac, mac_text);

  char* existing = hashmap_get(state->arp_cache, ip_key);
  if (existing != NULL) {
    snprintf(existing, 18U, "%s", mac_text);
    return MAGI_OK;
  }

  char* stored_mac = duplicate_string(mac_text);
  if (stored_mac == NULL) {
    return MAGI_ERR_NOMEM;
  }

  int status = hashmap_set(state->arp_cache, ip_key, stored_mac);
  if (status != MAGI_OK) {
    free(stored_mac);
    return status;
  }

  return MAGI_OK;
}

/**
 * Send an L2 Ethernet frame with the given payload via a specific interface.
 *
 * Builds an EthernetFrame with the specified destination MAC, source MAC
 * (from the interface), and ethertype, serializes it, and transmits it
 * through the interface.
 *
 * @param host        Pointer to the Host.
 * @param iface       Interface to transmit through.
 * @param dst_mac     Destination MAC address (6 bytes).
 * @param ethertype   Ethertype field value.
 * @param payload     Payload data (may be NULL if payload_len is 0).
 * @param payload_len Length of the payload in bytes.
 * @return MAGI_OK on success, or a negative error code on failure.
 */
static int host_send_ethernet_payload(Host* host, Interface* iface, const uint8_t dst_mac[6],
                                      uint16_t ethertype, const uint8_t* payload,
                                      size_t payload_len) {
  if (host == NULL || iface == NULL || dst_mac == NULL || (payload_len > 0U && payload == NULL)) {
    magi_errno = MAGI_ERR_BADARGS;
    return MAGI_ERR_BADARGS;
  }

  EthernetFrame frame = {0};
  memcpy(frame.dst_mac, dst_mac, ETHERNET_MAC_LEN);
  memcpy(frame.src_mac, iface->mac, ETHERNET_MAC_LEN);
  frame.ethertype = ethertype;
  frame.payload = payload;
  frame.payload_len = payload_len;

  uint8_t* bytes = NULL;
  size_t len = 0U;
  int status = ethernet_frame_to_bytes(&frame, &bytes, &len);
  if (status != MAGI_OK) {
    return status;
  }

  char dst_text[18];
  mac_to_str(dst_mac, dst_text);
  Node* node = host_as_node(host);
  LOG(node->name, "Send Ethernet frame dst=%s ethertype=0x%04X len=%zu", dst_text,
      (unsigned)ethertype, payload_len);

  status = interface_send(iface, bytes, len);
  return status;
}

/**
 * Build and send an ARP request for a target IP address.
 *
 * Constructs an ARP request message with the host's MAC and IP, serializes
 * it into an Ethernet frame addressed to the broadcast MAC, and transmits
 * it through the specified interface.
 *
 * @param host      Pointer to the Host.
 * @param iface     Interface to transmit the request through.
 * @param target_ip Target IP address as a dotted-decimal string.
 * @return MAGI_OK on success, or a negative error code on failure.
 */
static int host_send_arp_request(Host* host, Interface* iface, const char* target_ip) {
  HostState* state = host_state(host);
  if (state == NULL || iface == NULL || target_ip == NULL || state->ip_key[0] == '\0') {
    magi_errno = MAGI_ERR_BADARGS;
    return MAGI_ERR_BADARGS;
  }

  ARPMessage request = {0};
  request.opcode = ARP_OPCODE_REQUEST;
  memcpy(request.sender_mac, iface->mac, ETHERNET_MAC_LEN);
  if (arp_ipv4_from_string(state->ip_key, request.sender_ip) != MAGI_OK ||
      arp_ipv4_from_string(target_ip, request.target_ip) != MAGI_OK) {
    return MAGI_ERR_BADARGS;
  }

  uint8_t* arp_bytes = NULL;
  size_t arp_len = 0U;
  int status = arp_message_to_bytes(&request, &arp_bytes, &arp_len);
  if (status != MAGI_OK) {
    return status;
  }

  uint8_t broadcast[ETHERNET_MAC_LEN];
  ethernet_mac_broadcast(broadcast);
  LOG(host_as_node(host)->name, "ARP Miss for %s. Send ARP Request", target_ip);
  status =
      host_send_ethernet_payload(host, iface, broadcast, ETHERNET_TYPE_ARP, arp_bytes, arp_len);
  free(arp_bytes);
  return status;
}

/**
 * Queue an L3 packet for deferred transmission while ARP resolution is pending.
 *
 * Creates a PendingPacket entry and appends it to the linked list for the
 * given target IP. If no list exists yet, a new one is created and stored
 * in the pending hash map.
 *
 * @param host        Pointer to the Host.
 * @param target_ip   Target IP string to key the pending entry.
 * @param port        Port number for the outgoing interface.
 * @param ethertype   Ethertype of the queued packet.
 * @param payload     Payload data (may be NULL if payload_len is 0).
 * @param payload_len Length of the payload in bytes.
 * @return MAGI_OK on success, or a negative error code on failure.
 */
static int host_queue_pending(Host* host, const char* target_ip, uint16_t port, uint16_t ethertype,
                              const uint8_t* payload, size_t payload_len) {
  HostState* state = host_state(host);
  if (state == NULL || target_ip == NULL || (payload_len > 0U && payload == NULL)) {
    magi_errno = MAGI_ERR_BADARGS;
    return MAGI_ERR_BADARGS;
  }

  PendingPacket* packet = calloc(1U, sizeof(*packet));
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
  packet->ethertype = ethertype;
  packet->port = port;

  PendingPacket* head = hashmap_get(state->pending, target_ip);
  if (head == NULL) {
    int status = hashmap_set(state->pending, target_ip, packet);
    if (status != MAGI_OK) {
      free_pending_list(packet);
      return status;
    }
  } else {
    PendingPacket* tail = head;
    while (tail->next != NULL) {
      tail = tail->next;
    }
    tail->next = packet;
  }

  LOG(host_as_node(host)->name, "Queue packet for %s while ARP resolves", target_ip);
  return MAGI_OK;
}

/**
 * Flush and send all queued packets for a resolved IP address.
 *
 * Retrieves the pending packet list for the given target IP, removes it
 * from the pending hash map, and sends each queued packet as an Ethernet
 * frame addressed to the specified destination MAC.
 *
 * @param host      Pointer to the Host.
 * @param target_ip Target IP string whose pending queue to flush.
 * @param dst_mac   Resolved destination MAC address (6 bytes).
 * @return MAGI_OK on success, or a negative error code if any send fails.
 */
static int host_flush_pending(Host* host, const char* target_ip, const uint8_t dst_mac[6]) {
  HostState* state = host_state(host);
  if (state == NULL || target_ip == NULL || dst_mac == NULL) {
    magi_errno = MAGI_ERR_BADARGS;
    return MAGI_ERR_BADARGS;
  }

  PendingPacket* packet = hashmap_get(state->pending, target_ip);
  if (packet == NULL) {
    return MAGI_OK;
  }

  (void)hashmap_delete(state->pending, target_ip);
  Node* node = host_as_node(host);
  LOG(node->name, "Send queued packet(s) for %s", target_ip);

  int final_status = MAGI_OK;
  while (packet != NULL) {
    PendingPacket* next = packet->next;
    Interface* iface = node_get_interface(node, packet->port);
    if (iface != NULL) {
      int status = host_send_ethernet_payload(host, iface, dst_mac, packet->ethertype,
                                              packet->payload, packet->payload_len);
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
 * Handle an incoming ARP message received from the network.
 *
 * Parses the ARP payload from an Ethernet frame and dispatches based on
 * the opcode (request or reply). On any ARP message, the sender's MAC is
 * learned into the ARP cache. For requests targeting this host, an ARP
 * reply is sent. For replies, pending packets for that IP are flushed.
 *
 * @param host  Pointer to the Host.
 * @param iface Interface on which the frame was received.
 * @param frame Pointer to the parsed Ethernet frame containing the ARP payload.
 */
static void host_handle_arp(Host* host, Interface* iface, const EthernetFrame* frame) {
  HostState* state = host_state(host);
  if (state == NULL || iface == NULL || frame == NULL) {
    return;
  }

  ARPMessage message = {0};
  if (arp_message_from_bytes(frame->payload, frame->payload_len, &message) != MAGI_OK) {
    LOG(host_as_node(host)->name, "Drop malformed ARP payload");
    return;
  }

  char sender_ip[16];
  char target_ip[16];
  char sender_mac[18];
  arp_ipv4_to_string(message.sender_ip, sender_ip);
  arp_ipv4_to_string(message.target_ip, target_ip);
  mac_to_str(message.sender_mac, sender_mac);

  (void)host_cache_arp(host, message.sender_ip, message.sender_mac);

  if (message.opcode == ARP_OPCODE_REQUEST) {
    LOG(host_as_node(host)->name, "ARP Request received: who-has %s from %s", target_ip, sender_ip);
    if (state->ip_key[0] == '\0' || strcmp(target_ip, state->ip_key) != 0) {
      return;
    }

    ARPMessage reply = {0};
    reply.opcode = ARP_OPCODE_REPLY;
    memcpy(reply.sender_mac, iface->mac, ETHERNET_MAC_LEN);
    memcpy(reply.target_mac, message.sender_mac, ETHERNET_MAC_LEN);
    memcpy(reply.sender_ip, message.target_ip, 4U);
    memcpy(reply.target_ip, message.sender_ip, 4U);

    uint8_t* arp_bytes = NULL;
    size_t arp_len = 0U;
    if (arp_message_to_bytes(&reply, &arp_bytes, &arp_len) != MAGI_OK) {
      return;
    }

    LOG(host_as_node(host)->name, "Send ARP Reply to %s (%s)", sender_ip, sender_mac);
    (void)host_send_ethernet_payload(host, iface, message.sender_mac, ETHERNET_TYPE_ARP, arp_bytes,
                                     arp_len);
    free(arp_bytes);
    return;
  }

  if (message.opcode == ARP_OPCODE_REPLY) {
    LOG(host_as_node(host)->name, "ARP Reply received: %s is at %s", sender_ip, sender_mac);
    (void)host_flush_pending(host, sender_ip, message.sender_mac);
  }
}

/**
 * Handle an incoming Ethernet frame received on an interface.
 *
 * This is the top-level receive callback registered with the Node. It
 * parses the Ethernet frame, validates the destination MAC (must match
 * the interface or be broadcast), and dispatches to the appropriate
 * handler based on ethertype (ARP or IPv4). Unsupported ethertypes are
 * logged and dropped.
 *
 * @param node The Node (castable to Host) that received the frame.
 * @param iface Interface on which the frame arrived.
 * @param data  Pointer to the raw frame bytes.
 * @param len   Length of the raw frame.
 */
static void host_handle_receive(Node* node, Interface* iface, const uint8_t* data, size_t len) {
  Host* host = host_from_node(node);
  arena_reset(node->arena);
  EthernetFrame frame = {0};

  if (ethernet_frame_from_bytes(data, len, &frame) != MAGI_OK) {
    LOG(node->name, "Drop malformed Ethernet frame");
    return;
  }

  if (!ethernet_mac_equal(frame.dst_mac, iface->mac) && !ethernet_mac_is_broadcast(frame.dst_mac)) {
    return;
  }

  if (frame.ethertype == ETHERNET_TYPE_ARP) {
    host_handle_arp(host, iface, &frame);
    return;
  }

  if (frame.ethertype == ETHERNET_TYPE_IPV4) {
    if (node->handle_l3_packet != NULL) {
      node->handle_l3_packet(node, iface, frame.payload, frame.payload_len);
      return;
    }

    LOG(node->name, "IPv4 payload received (%zu bytes); L3 handler is not configured",
        frame.payload_len);
    return;
  }

  LOG(node->name, "Drop unsupported ethertype 0x%04X", (unsigned)frame.ethertype);
}

/**
 * Print a single ARP cache entry via the logging system.
 *
 * Callback for hashmap_foreach used by host_print_arp_cache. Each entry
 * is logged as "ARP <ip> -> <mac>".
 *
 * @param key   ARP cache key (IP string).
 * @param value ARP cache value (MAC string).
 * @param ctx   Pointer to a PrintArpCtx tracking host name and count.
 */
static void print_arp_entry(const char* key, void* value, void* ctx) {
  PrintArpCtx* state = ctx;
  const char* mac = value;
  LOG(state->host_name, "ARP %s -> %s", key, mac != NULL ? mac : "(null)");
  state->count++;
}

/**
 * Send an L3 packet through a host's L3 send path, triggered from a Node pointer.
 *
 * Wrapper that casts the Node back to a Host and delegates to
 * host_send_l3_packet. Used as the registered callback on the Node.
 *
 * @param node        Pointer to the Node (must be a Host).
 * @param next_hop_ip Next-hop IP as a string.
 * @param ethertype   Ethertype of the payload.
 * @param payload     Payload data.
 * @param payload_len Length of the payload.
 * @return MAGI_OK on success, or a negative error code.
 */
static int host_send_l3_from_node(Node* node, const char* next_hop_ip, uint16_t ethertype,
                                  const uint8_t* payload, size_t payload_len) {
  return host_send_l3_packet(host_from_node(node), next_hop_ip, ethertype, payload, payload_len);
}

Host* host_new(const char* name) {
  Node* node = node_new(name);
  if (node == NULL) {
    return NULL;
  }

  HostState* state = host_state_new();
  if (state == NULL) {
    node_free(node);
    return NULL;
  }

  node->data = state;
  node->data_free = host_state_free;
  node->handle_receive = host_handle_receive;
  node->send_l3_packet = host_send_l3_from_node;
  return host_from_node(node);
}

void host_free(Host* host) {
  node_free(host_as_node(host));
}

Node* host_as_node(Host* host) {
  return (Node*)host;
}

const Node* host_as_node_const(const Host* host) {
  return (const Node*)host;
}

Host* host_from_node(Node* node) {
  return (Host*)node;
}

const Host* host_from_node_const(const Node* node) {
  return (const Host*)node;
}

int host_configure(Host* host, const char* ip_address, const char* default_gateway) {
  HostState* state = host_state(host);
  if (host == NULL || state == NULL) {
    magi_errno = MAGI_ERR_BADARGS;
    return MAGI_ERR_BADARGS;
  }

  if (ip_address != NULL && ip_address[0] != '\0') {
    snprintf(state->ip_address, sizeof(state->ip_address), "%s", ip_address);
    int status = normalize_ip_key(ip_address, state->ip_key);
    if (status != MAGI_OK) {
      return status;
    }

    Interface* iface = node_add_interface(host_as_node(host), 1U);
    if (iface == NULL) {
      return MAGI_ERR_BADARGS;
    }
    snprintf(iface->ip_address, sizeof(iface->ip_address), "%s", ip_address);
  }

  if (default_gateway != NULL && default_gateway[0] != '\0') {
    snprintf(state->default_gateway, sizeof(state->default_gateway), "%s", default_gateway);
    snprintf(host_as_node(host)->default_gateway, sizeof(host_as_node(host)->default_gateway), "%s",
             default_gateway);
  }

  return MAGI_OK;
}

int host_send_l3_packet(Host* host, const char* target_ip, uint16_t ethertype,
                        const uint8_t* payload, size_t payload_len) {
  HostState* state = host_state(host);
  if (host == NULL || state == NULL || target_ip == NULL || (payload_len > 0U && payload == NULL)) {
    magi_errno = MAGI_ERR_BADARGS;
    return MAGI_ERR_BADARGS;
  }

  if (state->ip_key[0] == '\0') {
    LOG(host_as_node(host)->name, "Cannot send L2 packet: host IP address is not configured");
    magi_errno = MAGI_ERR_BADARGS;
    return MAGI_ERR_BADARGS;
  }

  char target_key[16];
  int status = normalize_ip_key(target_ip, target_key);
  if (status != MAGI_OK) {
    return status;
  }

  Node* node = host_as_node(host);
  Interface* iface = node_get_interface(node, 1U);
  if (iface == NULL) {
    iface = node_add_interface(node, 1U);
  }
  if (iface == NULL) {
    return MAGI_ERR_BADARGS;
  }
  if (iface->link == NULL) {
    LOG(node->name, "Cannot send L2 packet: interface 1 is not linked");
    magi_errno = MAGI_ERR_NOLINK;
    return MAGI_ERR_NOLINK;
  }

  char* mac_text = hashmap_get(state->arp_cache, target_key);
  if (mac_text != NULL) {
    uint8_t dst_mac[ETHERNET_MAC_LEN];
    if (mac_from_str(mac_text, dst_mac) != MAGI_OK) {
      return MAGI_ERR_BADARGS;
    }
    return host_send_ethernet_payload(host, iface, dst_mac, ethertype, payload, payload_len);
  }

  status =
      host_queue_pending(host, target_key, iface->port_number, ethertype, payload, payload_len);
  if (status != MAGI_OK) {
    return status;
  }

  return host_send_arp_request(host, iface, target_key);
}

int host_probe_l2(Host* host, const char* target_ip) {
  static const uint8_t payload[] = "M1-L2-PROBE";
  return host_send_l3_packet(host, target_ip, ETHERNET_TYPE_IPV4, payload, sizeof(payload) - 1U);
}

void host_print_arp_cache(const Host* host) {
  const HostState* state = host_state_const(host);
  if (host == NULL || state == NULL) {
    LOG("ARP", "Host ARP cache unavailable");
    return;
  }

  const Node* node = host_as_node_const(host);
  PrintArpCtx ctx = {.host_name = node->name, .count = 0U};
  hashmap_foreach(state->arp_cache, print_arp_entry, &ctx);
  if (ctx.count == 0U) {
    LOG(node->name, "ARP cache empty");
  }
}
