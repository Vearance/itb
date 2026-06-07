#define _POSIX_C_SOURCE 200809L

#include "ipv4.h"

#include "core/interface.h"
#include "layer3/icmp.h"
#include "utils/byteops.h"
#include "utils/log.h"
#include "utils/magi_error.h"

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

typedef struct HostIPv4State {
  uint16_t next_id;
  uint16_t next_seq;
  uint16_t echo_id;
  uint16_t pending_seq;
  uint16_t pending_id;
  uint8_t pending_target[4];
  struct timespec sent_at;
  bool awaiting;
  bool trace_mode;
  bool trace_done;
  bool trace_reached;
  uint8_t trace_ttl;
} HostIPv4State;

/**
 * @brief Convert a 4-byte IPv4 address to a 32-bit integer (host byte order).
 *
 * @param ip Pointer to a 4-byte IPv4 address.
 * @return The address as a uint32_t in host byte order.
 */
static uint32_t ipv4_to_u32(const uint8_t ip[4]) {
  return ((uint32_t)ip[0] << 24U) | ((uint32_t)ip[1] << 16U) | ((uint32_t)ip[2] << 8U) |
         (uint32_t)ip[3];
}

/**
 * @brief Convert a 32-bit integer (host byte order) to a 4-byte IPv4 address.
 *
 * @param value The uint32_t in host byte order.
 * @param ip    Output buffer for the 4-byte address.
 */
static void u32_to_ipv4(uint32_t value, uint8_t ip[4]) {
  ip[0] = (uint8_t)(value >> 24U);
  ip[1] = (uint8_t)(value >> 16U);
  ip[2] = (uint8_t)(value >> 8U);
  ip[3] = (uint8_t)value;
}

/**
 * @brief Compute a 32-bit subnet mask from a prefix length.
 *
 * Clamps prefix_len to the [0, 32] range.
 *
 * @param prefix_len Prefix length (0-32).
 * @return Subnet mask as a uint32_t in host byte order.
 */
static uint32_t prefix_to_mask_u32(int prefix_len) {
  if (prefix_len <= 0) {
    return 0U;
  }

  if (prefix_len >= 32) {
    return 0xFFFFFFFFU;
  }

  return 0xFFFFFFFFU << (32 - prefix_len);
}

/**
 * @brief Derive a deterministic echo identifier from a node name using FNV-1a.
 *
 * Used to correlate ICMP Echo Requests with Replies.
 *
 * @param node Node whose name is hashed.
 * @return A 16-bit echo identifier.
 */
static uint16_t node_echo_id(const Node* node) {
  uint32_t hash = 2166136261U;
  const char* text = node != NULL ? node->name : "host";

  while (*text != '\0') {
    hash ^= (uint8_t)*text;
    hash *= 16777619U;
    ++text;
  }

  return (uint16_t)(hash & 0xFFFFU);
}

/**
 * @brief Get the mutable HostIPv4State from a node.
 *
 * @param node The node whose L3 state to retrieve.
 * @return Pointer to HostIPv4State, or NULL if unavailable.
 */
static HostIPv4State* host_ipv4_state(Node* node) {
  return node != NULL ? (HostIPv4State*)node->l3_data : NULL;
}

/**
 * @brief Get the const HostIPv4State from a node.
 *
 * @param node The node whose L3 state to retrieve.
 * @return Pointer to const HostIPv4State, or NULL if unavailable.
 */
static const HostIPv4State* host_ipv4_state_const(const Node* node) {
  return node != NULL ? (const HostIPv4State*)node->l3_data : NULL;
}

/**
 * @brief Find the first interface that has an IPv4 address configured.
 *
 * Iterates the node's interface hashmap and returns the first interface
 * whose ip_address field is non-empty.
 *
 * @param node The node to search.
 * @return Pointer to the first configured Interface, or NULL.
 */
static Interface* first_ipv4_interface(Node* node) {
  if (node == NULL || node->interfaces == NULL) {
    return NULL;
  }

  for (size_t index = 0U; index < node->interfaces->capacity; ++index) {
    HashEntry* entry = &node->interfaces->entries[index];
    if (entry->key != NULL && !entry->tombstone) {
      Interface* iface = entry->value;
      if (iface != NULL && iface->ip_address[0] != '\0') {
        return iface;
      }
    }
  }

  return NULL;
}

/**
 * @brief Check whether an interface has a specific IPv4 address.
 *
 * Parses the interface's stored ip_address string and compares it bytewise.
 *
 * @param iface The interface to check.
 * @param ip    The IPv4 address to look for.
 * @return true if the interface carries the given IP.
 */
static bool interface_has_ip(const Interface* iface, const uint8_t ip[4]) {
  uint8_t iface_ip[4];
  if (iface == NULL || iface->ip_address[0] == '\0' || ip == NULL) {
    return false;
  }

  if (ipv4_parse_address(iface->ip_address, iface_ip) != MAGI_OK) {
    return false;
  }

  return ipv4_addr_equal(iface_ip, ip);
}

/**
 * @brief Check whether any interface on a node has a specific IPv4 address.
 *
 * @param node The node whose interfaces to search.
 * @param ip   The IPv4 address to look for.
 * @return true if any interface has the given IP.
 */
static bool node_has_ip(const Node* node, const uint8_t ip[4]) {
  if (node == NULL || node->interfaces == NULL || ip == NULL) {
    return false;
  }

  for (size_t index = 0U; index < node->interfaces->capacity; ++index) {
    const HashEntry* entry = &node->interfaces->entries[index];
    if (entry->key != NULL && !entry->tombstone && interface_has_ip(entry->value, ip)) {
      return true;
    }
  }

  return false;
}

/**
 * @brief Compute elapsed time in milliseconds since a given timestamp.
 *
 * Uses CLOCK_MONOTONIC. Returns 0.0 on error or if start is NULL.
 *
 * @param start The reference timestamp.
 * @return Elapsed time in milliseconds.
 */
static double elapsed_ms_since(const struct timespec* start) {
  struct timespec now;
  if (start == NULL || clock_gettime(CLOCK_MONOTONIC, &now) != 0) {
    return 0.0;
  }

  time_t sec = now.tv_sec - start->tv_sec;
  long nsec = now.tv_nsec - start->tv_nsec;
  if (nsec < 0) {
    --sec;
    nsec += 1000000000L;
  }

  return ((double)sec * 1000.0) + ((double)nsec / 1000000.0);
}

/**
 * @brief Start tracking a pending ping/traceroute echo request.
 *
 * Records the target IP, identifier, sequence number, and timestamp so
 * that a matching reply or error can be correlated.
 *
 * @param state      The per-host IPv4 state.
 * @param target_ip  The destination IP of the echo request.
 * @param id         The ICMP identifier.
 * @param seq        The ICMP sequence number.
 * @param trace_mode Whether this is part of a traceroute sequence.
 * @param ttl        The TTL used for this probe (relevant for traceroute).
 */
static void start_pending(HostIPv4State* state, const uint8_t target_ip[4], uint16_t id,
                          uint16_t seq, bool trace_mode, uint8_t ttl) {
  if (state == NULL || target_ip == NULL) {
    return;
  }

  state->pending_id = id;
  state->pending_seq = seq;
  memcpy(state->pending_target, target_ip, 4U);
  state->awaiting = true;
  state->trace_mode = trace_mode;
  state->trace_done = false;
  state->trace_reached = false;
  state->trace_ttl = ttl;
  (void)clock_gettime(CLOCK_MONOTONIC, &state->sent_at);
}

/**
 * @brief Check whether a received ICMP echo reply matches a pending request.
 *
 * @param state The per-host IPv4 state.
 * @param id    The ICMP identifier from the reply.
 * @param seq   The ICMP sequence number from the reply.
 * @return true if the state has a pending request with matching id and seq.
 */
static bool pending_matches(const HostIPv4State* state, uint16_t id, uint16_t seq) {
  return state != NULL && state->awaiting && state->pending_id == id && state->pending_seq == seq;
}

/**
 * @brief Determine the next-hop IP address for a given destination.
 *
 * If the destination is on the same subnet as the interface, the next hop
 * is the destination itself. Otherwise, the node's default gateway is used.
 *
 * @param node   The sending node.
 * @param iface  The egress interface.
 * @param dst_ip The destination IPv4 address.
 * @param out    Buffer (size 16) to write the next-hop IP string.
 * @return MAGI_OK on success, or an error code.
 */
static int choose_next_hop(const Node* node, const Interface* iface, const uint8_t dst_ip[4],
                           char out[16]) {
  uint8_t iface_ip[4];
  uint8_t network[4];
  uint8_t mask[4];
  int prefix_len = 0;

  if (node == NULL || iface == NULL || dst_ip == NULL || out == NULL ||
      iface->ip_address[0] == '\0') {
    magi_errno = MAGI_ERR_BADARGS;
    return MAGI_ERR_BADARGS;
  }

  int status = ipv4_parse_cidr(iface->ip_address, iface_ip, network, mask, &prefix_len);
  if (status != MAGI_OK) {
    return status;
  }

  if (ipv4_addr_in_network(dst_ip, network, mask)) {
    ipv4_address_to_string(dst_ip, out);
    return MAGI_OK;
  }

  if (node->default_gateway[0] == '\0') {
    magi_errno = MAGI_ERR_NOROUTE;
    return MAGI_ERR_NOROUTE;
  }

  uint8_t gateway_ip[4];
  status = ipv4_parse_address(node->default_gateway, gateway_ip);
  if (status != MAGI_OK) {
    return status;
  }

  ipv4_address_to_string(gateway_ip, out);
  return MAGI_OK;
}

/**
 * @brief Build and send an IPv4 packet from a host.
 *
 * Constructs an IPv4 header using the interface's source IP, resolves a
 * next hop, serialises the packet, and passes it to the L2 send callback.
 *
 * @param node        The sending node.
 * @param iface       The egress interface.
 * @param dst_ip      The destination IPv4 address.
 * @param ttl         The IP time-to-live.
 * @param protocol    The IP protocol number (1=ICMP, 6=TCP, 17=UDP).
 * @param payload     Pointer to the payload data.
 * @param payload_len Length of the payload.
 * @return MAGI_OK on success, or an error code.
 */
static int host_send_ipv4(Node* node, Interface* iface, const uint8_t dst_ip[4], uint8_t ttl,
                          uint8_t protocol, const uint8_t* payload, size_t payload_len) {
  HostIPv4State* state = host_ipv4_state(node);
  if (node == NULL || iface == NULL || dst_ip == NULL || (payload_len > 0U && payload == NULL) ||
      state == NULL || node->send_l3_packet == NULL) {
    magi_errno = MAGI_ERR_BADARGS;
    return MAGI_ERR_BADARGS;
  }

  uint8_t src_ip[4];
  if (ipv4_parse_address(iface->ip_address, src_ip) != MAGI_OK) {
    LOG(node->name, "Cannot send IPv4 packet: interface %u has no valid IP address",
        (unsigned)iface->port_number);
    return MAGI_ERR_BADARGS;
  }

  IPv4Packet pkt = {0};
  pkt.version_ihl = IPV4_VERSION_IHL;
  pkt.identification = state->next_id++;
  pkt.ttl = ttl;
  pkt.protocol = protocol;
  memcpy(pkt.src_ip, src_ip, 4U);
  memcpy(pkt.dst_ip, dst_ip, 4U);
  pkt.payload = payload;
  pkt.payload_len = payload_len;

  uint8_t* bytes = NULL;
  size_t bytes_len = 0U;
  int status = ipv4_packet_to_bytes(&pkt, &bytes, &bytes_len);
  if (status != MAGI_OK) {
    return status;
  }

  char dst_text[16];
  char next_hop[16];
  ipv4_address_to_string(dst_ip, dst_text);
  status = choose_next_hop(node, iface, dst_ip, next_hop);
  if (status != MAGI_OK) {
    LOG(node->name, "No route to %s (default gateway is not configured)", dst_text);
    free(bytes);
    return status;
  }

  LOG(node->name, "Send IPv4 dst=%s ttl=%u proto=%u via %s", dst_text, (unsigned)ttl,
      (unsigned)protocol, next_hop);
  status = node->send_l3_packet(node, next_hop, IPV4_ETHERTYPE, bytes, bytes_len);
  free(bytes);
  return status;
}

/**
 * @brief Build and send an ICMP Echo message over IPv4.
 *
 * Creates an ICMP message (Echo Request or Echo Reply), serialises it,
 * and sends it via host_send_ipv4.
 *
 * @param node        The sending node.
 * @param iface       The egress interface.
 * @param dst_ip      The destination IPv4 address.
 * @param ttl         The IP time-to-live.
 * @param type        ICMP type (Echo Request or Echo Reply).
 * @param id          ICMP identifier.
 * @param seq         ICMP sequence number.
 * @param payload     Pointer to the ICMP payload data.
 * @param payload_len Length of the ICMP payload.
 * @return MAGI_OK on success, or an error code.
 */
static int send_echo(Node* node, Interface* iface, const uint8_t dst_ip[4], uint8_t ttl,
                     uint8_t type, uint16_t id, uint16_t seq, const uint8_t* payload,
                     size_t payload_len) {
  ICMPMessage msg = {0};
  msg.type = type;
  msg.code = 0U;
  msg.identifier = id;
  msg.sequence = seq;
  msg.payload = payload;
  msg.payload_len = payload_len;

  uint8_t* icmp_bytes = NULL;
  size_t icmp_len = 0U;
  int status = icmp_message_to_bytes(&msg, &icmp_bytes, &icmp_len);
  if (status != MAGI_OK) {
    return status;
  }

  status = host_send_ipv4(node, iface, dst_ip, ttl, IPV4_PROTOCOL_ICMP, icmp_bytes, icmp_len);
  free(icmp_bytes);
  return status;
}

/**
 * @brief Handle a received ICMP Echo Reply by logging RTT and clearing state.
 *
 * Only processes the reply if it matches a pending echo request. If
 * traceroute mode is active, tracks completion.
 *
 * @param node The receiving node.
 * @param pkt  The IPv4 packet containing the echo reply.
 * @param msg  The parsed ICMP echo reply message.
 */
static void handle_echo_reply(Node* node, const IPv4Packet* pkt, const ICMPMessage* msg) {
  HostIPv4State* state = host_ipv4_state(node);
  if (!pending_matches(state, msg->identifier, msg->sequence)) {
    return;
  }

  double elapsed = elapsed_ms_since(&state->sent_at);
  char src_text[16];
  ipv4_address_to_string(pkt->src_ip, src_text);

  if (state->trace_mode) {
    LOG(node->name, "traceroute %u %s %.3f ms", (unsigned)state->trace_ttl, src_text, elapsed);
    state->trace_done = true;
    state->trace_reached = true;
  } else {
    LOG(node->name, "ping reply from %s: seq=%u ttl=%u time=%.3f ms", src_text,
        (unsigned)msg->sequence, (unsigned)pkt->ttl, elapsed);
  }

  state->awaiting = false;
}

/**
 * @brief Check whether an ICMP error (TTL exceeded / unreachable) embeds an
 *        echo request that matches a pending probe.
 *
 * The ICMP error payload contains the original IP header plus 8 bytes of
 * the original transport header. This function extracts that info and
 * compares it against the pending ping/traceroute state.
 *
 * @param state The per-host IPv4 state.
 * @param msg   The ICMP error message containing the embedded original.
 * @return true if the embedded echo matches the pending probe.
 */
static bool embedded_echo_matches(const HostIPv4State* state, const ICMPMessage* msg) {
  if (state == NULL || msg == NULL || msg->payload == NULL || msg->payload_len < 32U) {
    return false;
  }

  const uint8_t* original_ip = msg->payload + 4U;
  uint8_t protocol = original_ip[9];
  uint16_t original_total_len = READ_U16(original_ip, 2U);
  if (protocol != IPV4_PROTOCOL_ICMP || original_total_len < IPV4_HEADER_LEN + ICMP_HEADER_LEN) {
    return false;
  }

  const uint8_t* original_icmp = original_ip + IPV4_HEADER_LEN;
  uint8_t original_type = original_icmp[0];
  uint16_t original_id = READ_U16(original_icmp, 4U);
  uint16_t original_seq = READ_U16(original_icmp, 6U);
  return original_type == ICMP_TYPE_ECHO_REQUEST &&
         pending_matches(state, original_id, original_seq);
}

/**
 * @brief Handle an ICMP error message (Time Exceeded or Destination Unreachable).
 *
 * Checks whether the embedded original packet matches a pending probe and,
 * if so, logs the error with RTT information. For traceroute, tracks whether
 * the destination was reached or an error ended the probe.
 *
 * @param node The receiving node.
 * @param pkt  The IPv4 packet containing the ICMP error.
 * @param msg  The parsed ICMP error message.
 */
static void handle_icmp_error(Node* node, const IPv4Packet* pkt, const ICMPMessage* msg) {
  HostIPv4State* state = host_ipv4_state(node);
  if (!embedded_echo_matches(state, msg)) {
    return;
  }

  double elapsed = elapsed_ms_since(&state->sent_at);
  char src_text[16];
  ipv4_address_to_string(pkt->src_ip, src_text);

  if (state->trace_mode) {
    const char* suffix = msg->type == ICMP_TYPE_DEST_UNREACHABLE ? " !N" : "";
    LOG(node->name, "traceroute %u %s %.3f ms%s", (unsigned)state->trace_ttl, src_text, elapsed,
        suffix);
    state->trace_done = msg->type == ICMP_TYPE_DEST_UNREACHABLE;
    state->trace_reached = false;
  } else if (msg->type == ICMP_TYPE_TIME_EXCEEDED) {
    LOG(node->name, "ping time exceeded from %s", src_text);
  } else {
    LOG(node->name, "ping destination unreachable from %s", src_text);
  }

  state->awaiting = false;
}

/**
 * @brief Entry point for IPv4 packet reception on a host node.
 *
 * Unpacks the IPv4 header, discards packets not destined for this host,
 * and dispatches by protocol number. Non-ICMP protocols are forwarded to
 * the L4 handler if configured. ICMP messages are parsed further: Echo
 * Requests generate an Echo Reply, Echo Replies are correlated against
 * pending probes, and errors (Time Exceeded / Unreachable) are logged.
 *
 * @param node  The receiving node.
 * @param iface The interface on which the packet arrived.
 * @param data  The raw packet bytes (starting at the IPv4 header).
 * @param len   Length of the data.
 */
static void ipv4_host_receive(Node* node, Interface* iface, const uint8_t* data, size_t len) {
  if (node == NULL || iface == NULL || data == NULL) {
    return;
  }

  IPv4Packet pkt = {0};
  int status = ipv4_unpack(&pkt, data, len);
  if (status != MAGI_OK) {
    LOG(node->name, "Drop IPv4 packet: bad header/checksum");
    return;
  }

  if (!node_has_ip(node, pkt.dst_ip)) {
    return;
  }

  /* Non-ICMP protocol → dispatch to L4 handler if configured */
  if (pkt.protocol != IPV4_PROTOCOL_ICMP) {
    if (node->handle_l4_packet != NULL) {
      node->handle_l4_packet(node, pkt.src_ip, pkt.dst_ip, pkt.protocol, pkt.payload,
                             pkt.payload_len);
      return;
    }
    LOG(node->name, "Drop IPv4 protocol %u: no handler", (unsigned)pkt.protocol);
    return;
  }

  ICMPMessage msg = {0};
  if (icmp_unpack(&msg, pkt.payload, pkt.payload_len) != MAGI_OK) {
    LOG(node->name, "Drop ICMP packet: bad checksum");
    return;
  }

  char src_text[16];
  ipv4_address_to_string(pkt.src_ip, src_text);

  if (msg.type == ICMP_TYPE_ECHO_REQUEST) {
    LOG(node->name, "ICMP echo request from %s seq=%u", src_text, (unsigned)msg.sequence);
    (void)send_echo(node, iface, pkt.src_ip, IPV4_DEFAULT_TTL, ICMP_TYPE_ECHO_REPLY, msg.identifier,
                    msg.sequence, msg.payload, msg.payload_len);
    return;
  }

  if (msg.type == ICMP_TYPE_ECHO_REPLY) {
    handle_echo_reply(node, &pkt, &msg);
    return;
  }

  if (msg.type == ICMP_TYPE_TIME_EXCEEDED || msg.type == ICMP_TYPE_DEST_UNREACHABLE) {
    handle_icmp_error(node, &pkt, &msg);
  }
}

int ipv4_pack(IPv4Packet* pkt, uint8_t* out, size_t out_len) {
  if (pkt == NULL || out == NULL || (pkt->payload_len > 0U && pkt->payload == NULL)) {
    magi_errno = MAGI_ERR_BADARGS;
    return MAGI_ERR_BADARGS;
  }

  size_t total_len = IPV4_HEADER_LEN + pkt->payload_len;
  if (total_len > 0xFFFFU || out_len < total_len) {
    magi_errno = MAGI_ERR_BADARGS;
    return MAGI_ERR_BADARGS;
  }

  memset(out, 0, total_len);
  uint8_t version_ihl = pkt->version_ihl != 0U ? pkt->version_ihl : IPV4_VERSION_IHL;
  WRITE_U8(out, 0U, version_ihl);
  WRITE_U8(out, 1U, pkt->tos);
  WRITE_U16(out, 2U, (uint16_t)total_len);
  WRITE_U16(out, 4U, pkt->identification);
  WRITE_U16(out, 6U, pkt->flags_frag_off);
  WRITE_U8(out, 8U, pkt->ttl);
  WRITE_U8(out, 9U, pkt->protocol);
  WRITE_U16(out, 10U, 0U);
  memcpy(out + 12U, pkt->src_ip, 4U);
  memcpy(out + 16U, pkt->dst_ip, 4U);

  if (pkt->payload_len > 0U) {
    memcpy(out + IPV4_HEADER_LEN, pkt->payload, pkt->payload_len);
  }

  pkt->version_ihl = version_ihl;
  pkt->total_len = (uint16_t)total_len;
  pkt->checksum = ipv4_checksum(out, IPV4_HEADER_LEN);
  WRITE_U16(out, 10U, pkt->checksum);
  return MAGI_OK;
}

int ipv4_unpack(IPv4Packet* pkt, const uint8_t* in, size_t in_len) {
  if (pkt == NULL || in == NULL || in_len < IPV4_HEADER_LEN) {
    magi_errno = MAGI_ERR_BADARGS;
    return MAGI_ERR_BADARGS;
  }

  uint8_t version_ihl = in[0];
  uint8_t version = (uint8_t)(version_ihl >> 4U);
  uint8_t ihl = (uint8_t)(version_ihl & 0x0FU);
  if (version != 4U || ihl != 5U) {
    magi_errno = MAGI_ERR_BADARGS;
    return MAGI_ERR_BADARGS;
  }

  uint16_t total_len = READ_U16(in, 2U);
  if (total_len < IPV4_HEADER_LEN || total_len > in_len) {
    magi_errno = MAGI_ERR_BADARGS;
    return MAGI_ERR_BADARGS;
  }

  if (ipv4_checksum(in, IPV4_HEADER_LEN) != 0U) {
    magi_errno = MAGI_ERR_BADCKSUM;
    return MAGI_ERR_BADCKSUM;
  }

  memset(pkt, 0, sizeof(*pkt));
  pkt->version_ihl = version_ihl;
  pkt->tos = in[1];
  pkt->total_len = total_len;
  pkt->identification = READ_U16(in, 4U);
  pkt->flags_frag_off = READ_U16(in, 6U);
  pkt->ttl = in[8];
  pkt->protocol = in[9];
  pkt->checksum = READ_U16(in, 10U);
  memcpy(pkt->src_ip, in + 12U, 4U);
  memcpy(pkt->dst_ip, in + 16U, 4U);
  pkt->payload = in + IPV4_HEADER_LEN;
  pkt->payload_len = (size_t)total_len - IPV4_HEADER_LEN;
  return MAGI_OK;
}

int ipv4_packet_to_bytes(IPv4Packet* pkt, uint8_t** bytes_out, size_t* len_out) {
  if (pkt == NULL || bytes_out == NULL || len_out == NULL) {
    magi_errno = MAGI_ERR_BADARGS;
    return MAGI_ERR_BADARGS;
  }

  size_t total_len = IPV4_HEADER_LEN + pkt->payload_len;
  if (total_len > 0xFFFFU) {
    magi_errno = MAGI_ERR_BADARGS;
    return MAGI_ERR_BADARGS;
  }

  uint8_t* bytes = malloc(total_len);
  if (bytes == NULL) {
    magi_errno = MAGI_ERR_NOMEM;
    return MAGI_ERR_NOMEM;
  }

  int status = ipv4_pack(pkt, bytes, total_len);
  if (status != MAGI_OK) {
    free(bytes);
    return status;
  }

  *bytes_out = bytes;
  *len_out = total_len;
  return MAGI_OK;
}

/**
 * @brief Parse a CIDR-notation string ("a.b.c.d/prefix") and optionally
 *        extract the IP, network address, subnet mask, and prefix length.
 *
 * The prefix length is optional; if absent, /32 is assumed.
 *
 * @param text           The CIDR string to parse.
 * @param ip_out         Output buffer for the 4-byte IP address (may be NULL).
 * @param network_out    Output buffer for the network address (may be NULL).
 * @param mask_out       Output buffer for the subnet mask (may be NULL).
 * @param prefix_len_out Output pointer for the prefix length (may be NULL).
 * @return MAGI_OK on success, or MAGI_ERR_BADARGS.
 */
int ipv4_parse_cidr(const char* text, uint8_t ip_out[4], uint8_t network_out[4],
                    uint8_t mask_out[4], int* prefix_len_out) {
  if (text == NULL) {
    magi_errno = MAGI_ERR_BADARGS;
    return MAGI_ERR_BADARGS;
  }

  unsigned int octets[4] = {0U};
  int consumed = 0;
  if (sscanf(text, "%3u.%3u.%3u.%3u%n", &octets[0], &octets[1], &octets[2], &octets[3],
             &consumed) != 4 ||
      octets[0] > 255U || octets[1] > 255U || octets[2] > 255U || octets[3] > 255U) {
    magi_errno = MAGI_ERR_BADARGS;
    return MAGI_ERR_BADARGS;
  }

  int prefix_len = 32;
  const char* suffix = text + consumed;
  if (*suffix == '/') {
    char* end = NULL;
    long parsed_prefix = strtol(suffix + 1, &end, 10);
    if (end == suffix + 1 || *end != '\0' || parsed_prefix < 0 || parsed_prefix > 32) {
      magi_errno = MAGI_ERR_BADARGS;
      return MAGI_ERR_BADARGS;
    }
    prefix_len = (int)parsed_prefix;
  } else if (*suffix != '\0') {
    magi_errno = MAGI_ERR_BADARGS;
    return MAGI_ERR_BADARGS;
  }

  uint8_t ip[4] = {(uint8_t)octets[0], (uint8_t)octets[1], (uint8_t)octets[2], (uint8_t)octets[3]};
  uint32_t mask_value = prefix_to_mask_u32(prefix_len);
  uint32_t network_value = ipv4_to_u32(ip) & mask_value;

  if (ip_out != NULL) {
    memcpy(ip_out, ip, 4U);
  }
  if (network_out != NULL) {
    u32_to_ipv4(network_value, network_out);
  }
  if (mask_out != NULL) {
    u32_to_ipv4(mask_value, mask_out);
  }
  if (prefix_len_out != NULL) {
    *prefix_len_out = prefix_len;
  }

  return MAGI_OK;
}

/**
 * @brief Parse a plain dotted-decimal IPv4 address string ("a.b.c.d").
 *
 * Delegates to ipv4_parse_cidr without the CIDR suffix.
 *
 * @param text The dotted-decimal string.
 * @param out  Output buffer for the 4-byte IPv4 address.
 * @return MAGI_OK on success, or MAGI_ERR_BADARGS.
 */
int ipv4_parse_address(const char* text, uint8_t out[4]) {
  return ipv4_parse_cidr(text, out, NULL, NULL, NULL);
}

/**
 * @brief Format a 4-byte IPv4 address as a dotted-decimal string.
 *
 * Writes "a.b.c.d\0" into the 16-byte output buffer. If ip is NULL,
 * writes an empty string.
 *
 * @param ip  The 4-byte IPv4 address.
 * @param out Output buffer (must be at least 16 bytes).
 */
void ipv4_address_to_string(const uint8_t ip[4], char out[16]) {
  if (out == NULL) {
    return;
  }

  if (ip == NULL) {
    out[0] = '\0';
    return;
  }

  snprintf(out, 16U, "%u.%u.%u.%u", (unsigned)ip[0], (unsigned)ip[1], (unsigned)ip[2],
           (unsigned)ip[3]);
}

/**
 * @brief Format a network address and prefix length as a CIDR string.
 *
 * Writes "a.b.c.d/prefix\0" into the output buffer.
 *
 * @param network   The 4-byte network address.
 * @param prefix_len The prefix length (0-32).
 * @param out       Output buffer.
 * @param out_len   Size of the output buffer.
 * @return MAGI_OK on success, or MAGI_ERR_BADARGS.
 */
int ipv4_format_cidr(const uint8_t network[4], int prefix_len, char* out, size_t out_len) {
  if (network == NULL || out == NULL || out_len == 0U || prefix_len < 0 || prefix_len > 32) {
    magi_errno = MAGI_ERR_BADARGS;
    return MAGI_ERR_BADARGS;
  }

  int written = snprintf(out, out_len, "%u.%u.%u.%u/%d", (unsigned)network[0], (unsigned)network[1],
                         (unsigned)network[2], (unsigned)network[3], prefix_len);
  if (written < 0 || (size_t)written >= out_len) {
    magi_errno = MAGI_ERR_BADARGS;
    return MAGI_ERR_BADARGS;
  }

  return MAGI_OK;
}

/**
 * @brief Compare two 4-byte IPv4 addresses for equality.
 *
 * @param lhs First IPv4 address.
 * @param rhs Second IPv4 address.
 * @return true if both are non-NULL and byte-identical.
 */
bool ipv4_addr_equal(const uint8_t lhs[4], const uint8_t rhs[4]) {
  return lhs != NULL && rhs != NULL && memcmp(lhs, rhs, 4U) == 0;
}

/**
 * @brief Check whether an IPv4 address is all zeros (0.0.0.0).
 *
 * @param ip The IPv4 address to test.
 * @return true if all four bytes are zero.
 */
bool ipv4_addr_is_zero(const uint8_t ip[4]) {
  static const uint8_t zero[4] = {0U, 0U, 0U, 0U};
  return ipv4_addr_equal(ip, zero);
}

/**
 * @brief Check whether an IPv4 address belongs to a given network.
 *
 * Computes (ip & mask) and compares against the network address.
 *
 * @param ip      The IPv4 address to test.
 * @param network The network address.
 * @param mask    The subnet mask.
 * @return true if the IP is within the network.
 */
bool ipv4_addr_in_network(const uint8_t ip[4], const uint8_t network[4], const uint8_t mask[4]) {
  if (ip == NULL || network == NULL || mask == NULL) {
    return false;
  }

  return (ipv4_to_u32(ip) & ipv4_to_u32(mask)) == ipv4_to_u32(network);
}

int ipv4_host_attach(Node* node) {
  if (node == NULL) {
    magi_errno = MAGI_ERR_BADARGS;
    return MAGI_ERR_BADARGS;
  }

  if (node->l3_data != NULL) {
    node->handle_l3_packet = ipv4_host_receive;
    return MAGI_OK;
  }

  HostIPv4State* state = calloc(1U, sizeof(*state));
  if (state == NULL) {
    magi_errno = MAGI_ERR_NOMEM;
    return MAGI_ERR_NOMEM;
  }

  state->next_id = 1U;
  state->next_seq = 1U;
  state->echo_id = node_echo_id(node);
  node->l3_data = state;
  node->l3_data_free = free;
  node->handle_l3_packet = ipv4_host_receive;
  return MAGI_OK;
}

int ipv4_host_ping(Node* node, const char* target_ip) {
  static const uint8_t payload[] = "MAGI-PING";
  HostIPv4State* state = host_ipv4_state(node);
  if (node == NULL || target_ip == NULL || state == NULL) {
    magi_errno = MAGI_ERR_BADARGS;
    return MAGI_ERR_BADARGS;
  }

  Interface* iface = first_ipv4_interface(node);
  if (iface == NULL) {
    LOG(node->name, "ping: no IPv4 interface is configured");
    magi_errno = MAGI_ERR_BADARGS;
    return MAGI_ERR_BADARGS;
  }

  uint8_t dst_ip[4];
  int status = ipv4_parse_address(target_ip, dst_ip);
  if (status != MAGI_OK) {
    return status;
  }

  uint16_t seq = state->next_seq++;
  start_pending(state, dst_ip, state->echo_id, seq, false, IPV4_DEFAULT_TTL);
  status = send_echo(node, iface, dst_ip, IPV4_DEFAULT_TTL, ICMP_TYPE_ECHO_REQUEST, state->echo_id,
                     seq, payload, sizeof(payload) - 1U);
  if (status != MAGI_OK) {
    state->awaiting = false;
    return status;
  }

  if (host_ipv4_state_const(node)->awaiting) {
    LOG(node->name, "ping request for %s is pending", target_ip);
  }

  return MAGI_OK;
}

int ipv4_host_traceroute(Node* node, const char* target_ip, uint8_t max_hops) {
  static const uint8_t payload[] = "MAGI-TRACE";
  HostIPv4State* state = host_ipv4_state(node);
  if (node == NULL || target_ip == NULL || state == NULL || max_hops == 0U) {
    magi_errno = MAGI_ERR_BADARGS;
    return MAGI_ERR_BADARGS;
  }

  Interface* iface = first_ipv4_interface(node);
  if (iface == NULL) {
    LOG(node->name, "traceroute: no IPv4 interface is configured");
    magi_errno = MAGI_ERR_BADARGS;
    return MAGI_ERR_BADARGS;
  }

  uint8_t dst_ip[4];
  int status = ipv4_parse_address(target_ip, dst_ip);
  if (status != MAGI_OK) {
    return status;
  }

  char dst_text[16];
  ipv4_address_to_string(dst_ip, dst_text);
  LOG(node->name, "traceroute to %s, %u hops max", dst_text, (unsigned)max_hops);

  for (uint8_t ttl = 1U; ttl <= max_hops; ++ttl) {
    uint16_t seq = state->next_seq++;
    start_pending(state, dst_ip, state->echo_id, seq, true, ttl);
    status = send_echo(node, iface, dst_ip, ttl, ICMP_TYPE_ECHO_REQUEST, state->echo_id, seq,
                       payload, sizeof(payload) - 1U);
    if (status != MAGI_OK) {
      state->awaiting = false;
      return status;
    }

    if (state->awaiting) {
      LOG(node->name, "traceroute %u * * *", (unsigned)ttl);
      state->awaiting = false;
    }

    if (state->trace_done) {
      break;
    }
  }

  return MAGI_OK;
}

int ipv4_send_packet(Node* node, const uint8_t src_ip[4], const uint8_t dst_ip[4], uint8_t protocol,
                     uint8_t ttl, const uint8_t* data, size_t len) {
  if (node == NULL || src_ip == NULL || dst_ip == NULL || (len > 0U && data == NULL)) {
    magi_errno = MAGI_ERR_BADARGS;
    return MAGI_ERR_BADARGS;
  }

  Interface* iface = first_ipv4_interface(node);
  if (iface == NULL) {
    LOG(node->name, "Cannot send IPv4 packet: no interface configured");
    magi_errno = MAGI_ERR_BADARGS;
    return MAGI_ERR_BADARGS;
  }

  HostIPv4State* state = host_ipv4_state(node);
  if (state == NULL) {
    magi_errno = MAGI_ERR_BADARGS;
    return MAGI_ERR_BADARGS;
  }

  IPv4Packet pkt;
  memset(&pkt, 0, sizeof(pkt));
  pkt.version_ihl = IPV4_VERSION_IHL;
  pkt.identification = state->next_id++;
  pkt.ttl = ttl;
  pkt.protocol = protocol;
  memcpy(pkt.src_ip, src_ip, 4U);
  memcpy(pkt.dst_ip, dst_ip, 4U);
  pkt.payload = data;
  pkt.payload_len = len;

  uint8_t* bytes = NULL;
  size_t bytes_len = 0U;
  int status = ipv4_packet_to_bytes(&pkt, &bytes, &bytes_len);
  if (status != MAGI_OK) {
    return status;
  }

  char next_hop[16];
  status = choose_next_hop(node, iface, dst_ip, next_hop);
  if (status != MAGI_OK) {
    LOG(node->name, "No route to destination");
    free(bytes);
    return status;
  }

  status = node->send_l3_packet(node, next_hop, IPV4_ETHERTYPE, bytes, bytes_len);
  free(bytes);
  return status;
}
