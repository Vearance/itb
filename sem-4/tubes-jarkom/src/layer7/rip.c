#define _POSIX_C_SOURCE 200809L

/**
 * @file rip.c
 * @brief RIPv2 simplified implementation — Bellman-Ford over UDP port 520.
 *
 * Implements a distance-vector routing protocol on Router nodes.
 * The algorithm:
 *   1. On rip_send_update(), the router broadcasts its routing table
 *      (all entries with metric < RIP_INFINITY) to all neighbours.
 *   2. On rip_handle_message(), each received entry is evaluated:
 *        new_metric = min(current_metric, sender_metric + 1)
 *      If the route improves or the next_hop is the sender, update.
 *   3. If any route changes, a triggered update is sent.
 *
 * RIP state is stored in node->l4_data to avoid modifying the Router
 * struct. The router's local IPv4 handler dispatches UDP port 520 to
 * rip_handle_message() via the registered callback.
 */

#include "rip.h"

#include "core/interface.h"
#include "core/link.h"
#include "layer3/ipv4.h"
#include "layer3/router.h"
#include "layer4/udp.h"
#include "utils/byteops.h"
#include "utils/log.h"
#include "utils/magi_error.h"

#include <stdlib.h>
#include <string.h>

/* ─── RIP State ─── */

typedef struct RIPRouteRecord {
  uint8_t network[4];
  int prefix_len;
  uint8_t next_hop[4];
  uint16_t out_port;
  struct RIPRouteRecord* next;
} RIPRouteRecord;

/**
 * @brief Per-router RIP state stored in node->l4_data.
 */
typedef struct RIPState {
  /** Sequence number for triggered update detection (simplified). */
  uint16_t sequence;
  /** Whether RIP has been fully initialised. */
  bool active;
  /** Routes installed by RIP, used to avoid deleting static routes. */
  RIPRouteRecord* learned_routes;
  /** Connected routes advertised previously, used for link-down poisoning. */
  RIPRouteRecord* advertised_connected;
} RIPState;

/* ─── Forward declarations of internal helpers ─── */

static void rip_free_state(void* data);

/* ─── Message Building / Parsing ─── */

/**
 * @brief Build a single RIP entry into a buffer.
 *
 * @param out        Output buffer (must be at least RIP_ENTRY_SIZE bytes).
 * @param network    4-byte network address.
 * @param prefix_len Prefix length.
 * @param metric     Route metric.
 * @return Number of bytes written (RIP_ENTRY_SIZE).
 */
static size_t rip_encode_entry(uint8_t* out, const uint8_t network[4], int prefix_len,
                               uint8_t metric) {
  memcpy(out, network, 4U);
  out[4] = (uint8_t)prefix_len;
  out[5] = metric;
  return RIP_ENTRY_SIZE;
}

/**
 * @brief Decode one RIP entry from a buffer.
 *
 * @param in          Input buffer.
 * @param network_out Output 4-byte network address.
 * @param prefix_len_out Output prefix length.
 * @param metric_out  Output metric.
 * @return MAGI_OK on success, MAGI_ERR_BADARGS if invalid.
 */
static int rip_decode_entry(const uint8_t* in, uint8_t network_out[4], int* prefix_len_out,
                            uint8_t* metric_out) {
  if (in == NULL || network_out == NULL || prefix_len_out == NULL || metric_out == NULL) {
    return MAGI_ERR_BADARGS;
  }
  memcpy(network_out, in, 4U);
  *prefix_len_out = (int)in[4];
  *metric_out = in[5];

  if (*prefix_len_out < 0 || *prefix_len_out > 32) {
    return MAGI_ERR_BADARGS;
  }
  return MAGI_OK;
}

static bool rip_route_record_matches(const RIPRouteRecord* record, const uint8_t network[4],
                                     int prefix_len) {
  return record != NULL && network != NULL && record->prefix_len == prefix_len &&
         ipv4_addr_equal(record->network, network);
}

static RIPRouteRecord* rip_route_record_find(RIPRouteRecord* list, const uint8_t network[4],
                                             int prefix_len) {
  for (RIPRouteRecord* record = list; record != NULL; record = record->next) {
    if (rip_route_record_matches(record, network, prefix_len)) {
      return record;
    }
  }
  return NULL;
}

static int rip_route_record_upsert(RIPRouteRecord** list, const uint8_t network[4],
                                   int prefix_len, const uint8_t next_hop[4],
                                   uint16_t out_port) {
  if (list == NULL || network == NULL) {
    magi_errno = MAGI_ERR_BADARGS;
    return MAGI_ERR_BADARGS;
  }

  RIPRouteRecord* record = rip_route_record_find(*list, network, prefix_len);
  if (record == NULL) {
    record = calloc(1U, sizeof(*record));
    if (record == NULL) {
      magi_errno = MAGI_ERR_NOMEM;
      return MAGI_ERR_NOMEM;
    }
    record->next = *list;
    *list = record;
  }

  memcpy(record->network, network, 4U);
  record->prefix_len = prefix_len;
  if (next_hop != NULL) {
    memcpy(record->next_hop, next_hop, 4U);
  } else {
    memset(record->next_hop, 0, sizeof(record->next_hop));
  }
  record->out_port = out_port;
  return MAGI_OK;
}

static bool rip_route_record_remove(RIPRouteRecord** list, const uint8_t network[4],
                                    int prefix_len) {
  if (list == NULL || network == NULL) {
    return false;
  }

  RIPRouteRecord** cursor = list;
  while (*cursor != NULL) {
    RIPRouteRecord* record = *cursor;
    if (rip_route_record_matches(record, network, prefix_len)) {
      *cursor = record->next;
      free(record);
      return true;
    }
    cursor = &record->next;
  }
  return false;
}

static void rip_route_record_free_all(RIPRouteRecord* list) {
  while (list != NULL) {
    RIPRouteRecord* next = list->next;
    free(list);
    list = next;
  }
}

typedef struct RipFindRouteCtx {
  const uint8_t* network;
  int prefix_len;
  const RoutingTableEntry* route;
} RipFindRouteCtx;

static void rip_find_route_cb(const RoutingTableEntry* route, void* ctx) {
  RipFindRouteCtx* state = (RipFindRouteCtx*)ctx;
  if (state == NULL || state->route != NULL || route == NULL || state->network == NULL) {
    return;
  }

  if (route->prefix_len == state->prefix_len && ipv4_addr_equal(route->network, state->network)) {
    state->route = route;
  }
}

static const RoutingTableEntry* rip_find_exact_route(Router* router, const uint8_t network[4],
                                                     int prefix_len) {
  RipFindRouteCtx ctx;
  ctx.network = network;
  ctx.prefix_len = prefix_len;
  ctx.route = NULL;
  router_foreach_route(router, rip_find_route_cb, &ctx);
  return ctx.route;
}

static bool rip_connected_route_from_iface(const Interface* iface, RoutingTableEntry* out) {
  if (iface == NULL || out == NULL || iface->link == NULL || iface->ip_address[0] == '\0') {
    return false;
  }

  memset(out, 0, sizeof(*out));
  uint8_t iface_ip[4];
  if (ipv4_parse_cidr(iface->ip_address, iface_ip, out->network, out->mask, &out->prefix_len) !=
      MAGI_OK) {
    return false;
  }

  out->out_port = iface->port_number;
  out->metric = RIP_DEFAULT_METRIC;
  return true;
}

static bool rip_node_has_connected_route(Node* node, const uint8_t network[4], int prefix_len) {
  if (node == NULL || node->interfaces == NULL || network == NULL) {
    return false;
  }

  for (size_t index = 0U; index < node->interfaces->capacity; ++index) {
    HashEntry* entry = &node->interfaces->entries[index];
    if (entry->key == NULL || entry->tombstone) {
      continue;
    }

    RoutingTableEntry connected = {0};
    if (rip_connected_route_from_iface(entry->value, &connected) &&
        connected.prefix_len == prefix_len && ipv4_addr_equal(connected.network, network)) {
      return true;
    }
  }

  return false;
}

typedef struct RipBuildCtx {
  uint8_t* buf;
  size_t offset;
  size_t buf_cap;
} RipBuildCtx;

static bool rip_build_add_entry(RipBuildCtx* ctx, const uint8_t network[4], int prefix_len,
                                uint8_t metric) {
  if (ctx == NULL || ctx->buf == NULL || network == NULL ||
      ctx->offset + RIP_ENTRY_SIZE > ctx->buf_cap) {
    return false;
  }

  uint8_t stored_metric = metric > RIP_INFINITY ? RIP_INFINITY : metric;
  ctx->offset += rip_encode_entry(ctx->buf + ctx->offset, network, prefix_len, stored_metric);
  return true;
}

/* ─── Internal: send a RIP message directly to a neighbour ─── */

/**
 * @brief Send a pre-built RIP message to a specific neighbour.
 *
 * Constructs a UDP/IPv4 packet with the RIP payload and sends it
 * via the router's forwarding path. The source IP is the router's
 * interface IP on the egress subnet.
 *
 * @param node      Router node.
 * @param dst_ip    Destination IPv4 address.
 * @param rip_msg   RIP message payload.
 * @param rip_len   RIP message length.
 * @return MAGI_OK on success, otherwise an error code.
 */
static int rip_send_to(Node* node, const uint8_t dst_ip[4], const uint8_t* rip_msg,
                       size_t rip_len) {
  Router* router = router_from_node(node);
  if (router == NULL || node == NULL || dst_ip == NULL || (rip_len > 0U && rip_msg == NULL)) {
    return MAGI_ERR_BADARGS;
  }

  /* Find the source IP for this destination via LPM */
  const RoutingTableEntry* route = lpm_lookup(router, dst_ip);
  uint8_t src_ip[4] = {0, 0, 0, 0};

  if (route != NULL) {
    Interface* egress = node_get_interface(node, route->out_port);
    if (egress != NULL && egress->ip_address[0] != '\0') {
      (void)ipv4_parse_address(egress->ip_address, src_ip);
    }
  }

  /* If no route, try the first configured interface */
  if (ipv4_addr_is_zero(src_ip)) {
    for (size_t i = 0U; i < node->interfaces->capacity; ++i) {
      HashEntry* entry = &node->interfaces->entries[i];
      if (entry->key != NULL && !entry->tombstone) {
        Interface* iface = (Interface*)entry->value;
        if (iface->ip_address[0] != '\0') {
          (void)ipv4_parse_address(iface->ip_address, src_ip);
          break;
        }
      }
    }
  }

  if (ipv4_addr_is_zero(src_ip)) {
    LOG(node->name, "RIP: cannot send update — no source IP found");
    return MAGI_ERR_BADARGS;
  }

  /* Build UDP datagram */
  UDPDatagram dgram;
  memset(&dgram, 0, sizeof(dgram));
  dgram.src_port = RIP_PORT;
  dgram.dst_port = RIP_PORT;
  dgram.payload = rip_msg;
  dgram.payload_len = rip_len;

  size_t udp_total = UDP_HEADER_LEN + rip_len;
  uint8_t* udp_buf = malloc(udp_total);
  if (udp_buf == NULL) {
    return MAGI_ERR_NOMEM;
  }

  int status = udp_pack(&dgram, src_ip, dst_ip, udp_buf, udp_total);
  if (status != MAGI_OK) {
    free(udp_buf);
    return status;
  }

  /* Build IPv4 packet */
  IPv4Packet pkt;
  memset(&pkt, 0, sizeof(pkt));
  pkt.version_ihl = IPV4_VERSION_IHL;
  pkt.identification = (uint16_t)(rand() & 0xFFFF);
  pkt.ttl = IPV4_DEFAULT_TTL;
  pkt.protocol = IPV4_PROTOCOL_UDP;
  memcpy(pkt.src_ip, src_ip, 4U);
  memcpy(pkt.dst_ip, dst_ip, 4U);
  pkt.payload = udp_buf;
  pkt.payload_len = udp_total;

  char dst_text[16];
  ipv4_address_to_string(dst_ip, dst_text);
  LOG(node->name, "RIP: send update to %s (%zu routes)", dst_text, rip_len / RIP_ENTRY_SIZE);

  status = router_send_ipv4(router, &pkt);
  free(udp_buf);
  return status;
}

/* ─── Collect routing entries callback ─── */

/**
 * @brief Context for collecting routes into a RIP message buffer.
 */
typedef struct {
  RipBuildCtx* build;
} RipCollectCtx;

/**
 * @brief Callback for router_foreach_route: add one route to the RIP message buffer.
 *
 * Skips routes with metric >= RIP_INFINITY (16 = unreachable) and
 * routes whose next_hop matches the sender (to avoid echo).
 * Encodes the remaining routes as RIP entries.
 */
static void rip_collect_route(const RoutingTableEntry* route, void* ctx) {
  RipCollectCtx* c = (RipCollectCtx*)ctx;
  if (c == NULL || c->build == NULL || route == NULL) {
    return;
  }

  if (route->metric >= RIP_INFINITY) {
    return;
  }

  (void)rip_build_add_entry(c->build, route->network, route->prefix_len, route->metric);
}

static int rip_replace_advertised_connected(RIPState* state, const RoutingTableEntry* routes,
                                            size_t count) {
  if (state == NULL || (count > 0U && routes == NULL)) {
    magi_errno = MAGI_ERR_BADARGS;
    return MAGI_ERR_BADARGS;
  }

  RIPRouteRecord* new_list = NULL;
  for (size_t index = 0U; index < count; ++index) {
    int status = rip_route_record_upsert(&new_list, routes[index].network, routes[index].prefix_len,
                                         NULL, routes[index].out_port);
    if (status != MAGI_OK) {
      rip_route_record_free_all(new_list);
      return status;
    }
  }

  rip_route_record_free_all(state->advertised_connected);
  state->advertised_connected = new_list;
  return MAGI_OK;
}

/* ─── Public API ─── */

int rip_init(Node* node) {
  if (node == NULL) {
    magi_errno = MAGI_ERR_BADARGS;
    return MAGI_ERR_BADARGS;
  }

  Router* router = router_from_node(node);
  if (router == NULL) {
    LOG(node->name, "RIP: node is not a router");
    magi_errno = MAGI_ERR_BADARGS;
    return MAGI_ERR_BADARGS;
  }

  /* Allocate RIP state if not already present */
  if (node->l4_data != NULL) {
    /* Already initialised — just ensure the handler is registered */
    node->async_tick_30s = rip_send_update;
    router_set_rip_handler(router, (rip_dispatch_fn)rip_handle_message);
    return MAGI_OK;
  }

  RIPState* state = calloc(1U, sizeof(*state));
  if (state == NULL) {
    magi_errno = MAGI_ERR_NOMEM;
    return MAGI_ERR_NOMEM;
  }

  state->sequence = 0U;
  state->active = true;

  node->l4_data = state;
  node->l4_data_free = rip_free_state;
  node->async_tick_30s = rip_send_update;

  /* Register the RIP message handler on the router */
  router_set_rip_handler(router, (rip_dispatch_fn)rip_handle_message);

  LOG(node->name, "RIP: initialised");
  return MAGI_OK;
}

int rip_send_update(Node* node) {
  if (node == NULL) {
    magi_errno = MAGI_ERR_BADARGS;
    return MAGI_ERR_BADARGS;
  }

  Router* router = router_from_node(node);
  if (router == NULL) {
    LOG(node->name, "RIP: node is not a router");
    return MAGI_ERR_BADARGS;
  }

  RIPState* state = (RIPState*)node->l4_data;
  if (state == NULL || !state->active) {
    LOG(node->name, "RIP: not initialised (call rip_init first)");
    return MAGI_ERR_BADARGS;
  }

  uint8_t msg_buf[RIP_HEADER_SIZE + RIP_MAX_ENTRIES * RIP_ENTRY_SIZE];
  RipBuildCtx build;
  build.buf = msg_buf + RIP_HEADER_SIZE;
  build.offset = 0U;
  build.buf_cap = RIP_MAX_ENTRIES * RIP_ENTRY_SIZE;

  RoutingTableEntry connected_routes[RIP_MAX_ENTRIES];
  size_t connected_count = 0U;

  for (size_t index = 0U; index < node->interfaces->capacity; ++index) {
    HashEntry* entry = &node->interfaces->entries[index];
    if (entry->key == NULL || entry->tombstone) {
      continue;
    }

    RoutingTableEntry connected = {0};
    if (!rip_connected_route_from_iface(entry->value, &connected)) {
      continue;
    }

    bool duplicate = false;
    for (size_t current = 0U; current < connected_count; ++current) {
      if (connected_routes[current].prefix_len == connected.prefix_len &&
          ipv4_addr_equal(connected_routes[current].network, connected.network)) {
        duplicate = true;
        break;
      }
    }

    if (duplicate || connected_count >= RIP_MAX_ENTRIES) {
      continue;
    }

    connected_routes[connected_count++] = connected;
    (void)rip_build_add_entry(&build, connected.network, connected.prefix_len, connected.metric);
  }

  for (const RIPRouteRecord* old = state->advertised_connected; old != NULL; old = old->next) {
    bool still_connected = false;
    for (size_t current = 0U; current < connected_count; ++current) {
      if (connected_routes[current].prefix_len == old->prefix_len &&
          ipv4_addr_equal(connected_routes[current].network, old->network)) {
        still_connected = true;
        break;
      }
    }

    if (!still_connected) {
      (void)rip_build_add_entry(&build, old->network, old->prefix_len, RIP_INFINITY);
    }
  }

  RipCollectCtx ctx;
  ctx.build = &build;
  router_foreach_route(router, rip_collect_route, &ctx);

  size_t num_entries = build.offset / RIP_ENTRY_SIZE;
  if (num_entries == 0U) {
    LOG(node->name, "RIP: no routes to advertise");
    return MAGI_OK;
  }

  /* Write RIP header */
  msg_buf[0] = RIP_RESPONSE;
  msg_buf[1] = (uint8_t)num_entries;

  size_t msg_len = RIP_HEADER_SIZE + num_entries * RIP_ENTRY_SIZE;

  int connected_status = rip_replace_advertised_connected(state, connected_routes, connected_count);
  if (connected_status != MAGI_OK) {
    return connected_status;
  }

  /* Send RIP update to all directly connected neighbours */
  int final_status = MAGI_OK;
  size_t sent_count = 0U;

  for (size_t i = 0U; i < node->interfaces->capacity; ++i) {
    HashEntry* entry = &node->interfaces->entries[i];
    if (entry->key == NULL || entry->tombstone) {
      continue;
    }

    Interface* iface = (Interface*)entry->value;
    if (iface == NULL || iface->link == NULL || iface->ip_address[0] == '\0') {
      continue;
    }

    /* Find the neighbour on the other end of this link */
    Link* link = iface->link;
    Interface* other_end = (link->endpoint_a == iface) ? link->endpoint_b : link->endpoint_a;

    if (other_end == NULL || other_end->node == NULL || other_end->node == node ||
        other_end->node->handle_receive != router_handle_receive) {
      continue;
    }

    /* Get the neighbour's IP from its interface */
    uint8_t neighbor_ip[4];
    if (other_end->ip_address[0] != '\0') {
      if (ipv4_parse_address(other_end->ip_address, neighbor_ip) != MAGI_OK) {
        continue;
      }
    } else {
      /* Try the neighbour node's first IPv4 interface */
      Node* neighbor_node = other_end->node;
      bool found = false;
      for (size_t j = 0U; j < neighbor_node->interfaces->capacity; ++j) {
        HashEntry* ne = &neighbor_node->interfaces->entries[j];
        if (ne->key != NULL && !ne->tombstone) {
          Interface* ni = (Interface*)ne->value;
          if (ni->ip_address[0] != '\0' &&
              ipv4_parse_address(ni->ip_address, neighbor_ip) == MAGI_OK) {
            found = true;
            break;
          }
        }
      }
      if (!found) {
        continue;
      }
    }

    /* Skip sending to self */
    uint8_t iface_ip[4];
    if (ipv4_parse_address(iface->ip_address, iface_ip) == MAGI_OK &&
        ipv4_addr_equal(iface_ip, neighbor_ip)) {
      continue;
    }

    int status = rip_send_to(node, neighbor_ip, msg_buf, msg_len);
    if (status != MAGI_OK) {
      final_status = status;
    } else {
      sent_count++;
    }
  }

  if (sent_count > 0U) {
    LOG(node->name, "RIP: sent update to %zu neighbour(s) (%zu routes)", sent_count, num_entries);
  } else {
    LOG(node->name, "RIP: no neighbours to send update to");
  }

  return final_status;
}

bool rip_is_active(const Node* node) {
  const RIPState* state = node != NULL ? (const RIPState*)node->l4_data : NULL;
  return state != NULL && state->active;
}

int rip_handle_link_down(Node* node, uint16_t port) {
  if (node == NULL || port == 0U) {
    magi_errno = MAGI_ERR_BADARGS;
    return MAGI_ERR_BADARGS;
  }

  RIPState* state = (RIPState*)node->l4_data;
  Router* router = router_from_node(node);
  if (state == NULL || !state->active || router == NULL) {
    return MAGI_OK;
  }

  RIPRouteRecord** cursor = &state->learned_routes;
  size_t removed = 0U;
  while (*cursor != NULL) {
    RIPRouteRecord* record = *cursor;
    if (record->out_port == port) {
      char dest_cidr[32];
      if (ipv4_format_cidr(record->network, record->prefix_len, dest_cidr, sizeof(dest_cidr)) ==
          MAGI_OK) {
        (void)router_remove_route(router, dest_cidr);
        LOG(node->name, "RIP: removed learned route %s after port %u went down", dest_cidr,
            (unsigned)port);
      }
      *cursor = record->next;
      free(record);
      removed++;
      continue;
    }
    cursor = &record->next;
  }

  if (removed > 0U) {
    (void)rip_send_update(node);
  }

  return MAGI_OK;
}

void rip_handle_message(Node* node, const uint8_t* data, size_t len, const uint8_t sender_ip[4]) {
  if (node == NULL || data == NULL || sender_ip == NULL) {
    return;
  }

  Router* router = router_from_node(node);
  if (router == NULL) {
    return;
  }

  RIPState* state = (RIPState*)node->l4_data;
  if (state == NULL || !state->active) {
    return;
  }

  /* Validate message header */
  if (len < RIP_HEADER_SIZE) {
    LOG(node->name, "RIP: received malformed message (too short)");
    return;
  }

  uint8_t op = data[0];
  uint8_t num_entries = data[1];

  if (op != RIP_RESPONSE) {
    /* Ignore non-response messages */
    return;
  }

  size_t expected_len = RIP_HEADER_SIZE + (size_t)num_entries * RIP_ENTRY_SIZE;
  if (expected_len > len) {
    LOG(node->name, "RIP: received truncated message (expected %zu, got %zu)", expected_len, len);
    return;
  }

  char sender_text[16];
  ipv4_address_to_string(sender_ip, sender_text);
  LOG(node->name, "RIP: received update from %s (%u entries)", sender_text, (unsigned)num_entries);

  /* Find the interface whose IP matches the sender's network (for out_port) */
  uint16_t sender_port = 0U;
  for (size_t i = 0U; i < node->interfaces->capacity; ++i) {
    HashEntry* entry = &node->interfaces->entries[i];
    if (entry->key == NULL || entry->tombstone) {
      continue;
    }
    Interface* iface = (Interface*)entry->value;
    if (iface == NULL || iface->ip_address[0] == '\0') {
      continue;
    }

    uint8_t iface_ip[4];
    uint8_t network[4];
    uint8_t mask[4];
    int prefix_len = 0;
    if (ipv4_parse_cidr(iface->ip_address, iface_ip, network, mask, &prefix_len) != MAGI_OK) {
      continue;
    }

    if (ipv4_addr_in_network(sender_ip, network, mask)) {
      sender_port = iface->port_number;
      break;
    }
  }

  if (sender_port == 0U) {
    LOG(node->name, "RIP: sender %s is not on any directly connected network", sender_text);
    return;
  }

  /* Process each entry with Bellman-Ford */
  bool route_changed = false;
  size_t updates = 0U;

  for (uint8_t e = 0U; e < num_entries; ++e) {
    size_t entry_off = RIP_HEADER_SIZE + (size_t)e * RIP_ENTRY_SIZE;
    uint8_t network[4];
    int prefix_len = 0;
    uint8_t advertised_metric = 0;

    if (rip_decode_entry(data + entry_off, network, &prefix_len, &advertised_metric) != MAGI_OK) {
      continue;
    }

    /* Compute new metric via this neighbour */
    uint8_t new_metric = advertised_metric + 1U;
    if (new_metric > RIP_INFINITY) {
      new_metric = RIP_INFINITY;
    }

    /* Build CIDR string for the destination network */
    char dest_cidr[32];
    if (ipv4_format_cidr(network, prefix_len, dest_cidr, sizeof(dest_cidr)) != MAGI_OK) {
      continue;
    }

    if (rip_node_has_connected_route(node, network, prefix_len)) {
      continue;
    }

    const RoutingTableEntry* exact = rip_find_exact_route(router, network, prefix_len);
    RIPRouteRecord* learned =
        rip_route_record_find(state->learned_routes, network, prefix_len);
    bool learned_same_next_hop =
        learned != NULL && ipv4_addr_equal(learned->next_hop, sender_ip);

    if (new_metric >= RIP_INFINITY) {
      if (learned_same_next_hop) {
        uint8_t old_metric = exact != NULL ? exact->metric : RIP_INFINITY;
        (void)router_remove_route(router, dest_cidr);
        (void)rip_route_record_remove(&state->learned_routes, network, prefix_len);
        LOG(node->name, "RIP: route %s unreachable (metric %u -> INF)", dest_cidr,
            (unsigned)old_metric);
        route_changed = true;
        updates++;
      }
      continue;
    }

    char next_hop_str[16];
    ipv4_address_to_string(sender_ip, next_hop_str);

    if (exact == NULL) {
      int status =
          router_add_route_metric(router, dest_cidr, next_hop_str, sender_port, new_metric);
      if (status == MAGI_OK &&
          rip_route_record_upsert(&state->learned_routes, network, prefix_len, sender_ip,
                                  sender_port) == MAGI_OK) {
        LOG(node->name, "RIP: new route %s via %s metric %u", dest_cidr, next_hop_str,
            (unsigned)new_metric);
        route_changed = true;
        updates++;
      }
      continue;
    }

    bool better_metric = new_metric < exact->metric;
    if (learned_same_next_hop || better_metric) {
      uint8_t old_metric = exact->metric;
      bool next_hop_changed = !ipv4_addr_equal(exact->next_hop, sender_ip);
      bool port_changed = exact->out_port != sender_port;

      if (old_metric != new_metric || next_hop_changed || port_changed) {
        int status =
            router_add_route_metric(router, dest_cidr, next_hop_str, sender_port, new_metric);
        if (status == MAGI_OK &&
            rip_route_record_upsert(&state->learned_routes, network, prefix_len, sender_ip,
                                    sender_port) == MAGI_OK) {
          LOG(node->name, "RIP: update %s metric %u -> %u via %s", dest_cidr,
              (unsigned)old_metric, (unsigned)new_metric, next_hop_str);
          route_changed = true;
          updates++;
        }
      }
    }
    continue;

    /* Check if we already have a route for this destination */
    uint8_t dst_ip[4];
    memcpy(dst_ip, network, 4U);

    const RoutingTableEntry* existing = lpm_lookup(router, dst_ip);

    if (existing == NULL) {
      /* New route — add it if metric is below infinity */
      if (new_metric < RIP_INFINITY) {
        char next_hop_str[16];
        ipv4_address_to_string(sender_ip, next_hop_str);
        int status = router_add_route(router, dest_cidr, next_hop_str, sender_port);
        if (status == MAGI_OK) {
          char net_str[16];
          ipv4_address_to_string(network, net_str);
          LOG(node->name, "RIP: new route %s via %s metric %u", dest_cidr, next_hop_str,
              (unsigned)new_metric);
          route_changed = true;
          updates++;
        }
      }
    } else {
      /* Existing route — apply Bellman-Ford update rule:
         1. If the existing next_hop is the sender, always update (even if metric increases)
         2. If the new metric is better, update */
      bool same_next_hop =
          ipv4_addr_equal(existing->next_hop, sender_ip) ||
          (!ipv4_addr_is_zero(existing->next_hop) && !ipv4_addr_is_zero(sender_ip) &&
           ipv4_addr_equal(existing->next_hop, sender_ip));
      bool better_metric = new_metric < existing->metric;

      if (same_next_hop || better_metric) {
        uint8_t old_metric = existing->metric;
        uint8_t effective_metric = same_next_hop ? new_metric : new_metric;

        if (effective_metric != old_metric &&
            !(same_next_hop && new_metric >= RIP_INFINITY && old_metric >= RIP_INFINITY)) {
          /* Update the route */
          router_remove_route(router, dest_cidr);
          char next_hop_str[16];
          ipv4_address_to_string(sender_ip, next_hop_str);

          if (effective_metric < RIP_INFINITY) {
            int status = router_add_route(router, dest_cidr, next_hop_str, sender_port);
            if (status == MAGI_OK) {
              if (effective_metric != old_metric) {
                LOG(node->name, "RIP: update %s metric %u → %u via %s", dest_cidr,
                    (unsigned)old_metric, (unsigned)effective_metric, next_hop_str);
                route_changed = true;
                updates++;
              }
            }
          } else {
            LOG(node->name, "RIP: route %s unreachable (metric %u → INF)", dest_cidr,
                (unsigned)old_metric);
            route_changed = true;
            updates++;
          }
        }
      }
    }
  }

  LOG(node->name, "RIP: processed %u entries from %s (%zu updates)", (unsigned)num_entries,
      sender_text, updates);

  /* Send triggered update if routes changed */
  if (route_changed) {
    LOG(node->name, "RIP: sending triggered update (%zu routes changed)", updates);
    (void)rip_send_update(node);
  }

  return;
}

/* ─── Private helpers ─── */

/**
 * @brief Free the RIP state attached to a node.
 *
 * Callback invoked via node->l4_data_free when the node is destroyed.
 *
 * @param data Pointer to the RIPState to free.
 */
static void rip_free_state(void* data) {
  RIPState* state = (RIPState*)data;
  if (state == NULL) {
    return;
  }
  rip_route_record_free_all(state->learned_routes);
  rip_route_record_free_all(state->advertised_connected);
  free(state);
}
