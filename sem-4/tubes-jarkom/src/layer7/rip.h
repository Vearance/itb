/**
 * @file rip.h
 * @brief RIPv2 simplified implementation over UDP broadcast port 520.
 *
 * Uses Bellman-Ford distance-vector routing. Routers periodically or
 * on-demand send their routing tables (metric < INFINITY) as RIP
 * response messages to neighbouring routers. Received updates are
 * processed with the Bellman-Ford algorithm: if the metric via the
 * sender + 1 is lower than the current route, the route is updated.
 *
 * All application-layer protocols use only the MagiSocket API.
 * However, RIP operates on Router nodes which do not have MagiSocket
 * support. Instead, RIP integrates directly with the router's
 * forwarding path via router_set_rip_handler() and uses raw UDP/IPv4
 * packet construction through router_send_ipv4().
 */

#ifndef MAGI_LAYER7_RIP_H
#define MAGI_LAYER7_RIP_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "core/node.h"

/* ─── Constants ─── */

/** UDP port used by RIP. */
#define RIP_PORT 520U

/** Infinity metric — routes with this metric are considered unreachable. */
#define RIP_INFINITY 16U

/** Default metric for directly connected routes. */
#define RIP_DEFAULT_METRIC 1U

/** RIP message types. */
#define RIP_REQUEST 1U  /* request for routing information */
#define RIP_RESPONSE 2U /* response containing routing entries */

/**
 * @brief Simplified RIP entry wire format size.
 *
 * Each entry: network[4] | prefix_len[1] | metric[1] = 6 bytes.
 */
#define RIP_ENTRY_SIZE 6U

/**
 * @brief RIP message header size.
 *
 * op[1] | num_entries[1] = 2 bytes.
 */
#define RIP_HEADER_SIZE 2U

/** Maximum number of RIP entries per message (conservative estimate). */
#define RIP_MAX_ENTRIES 128U

/* ─── Functions ─── */

/**
 * @brief Initialise RIP on a router node.
 *
 * Registers the RIP dispatch handler on the router so that incoming
 * UDP packets to port 520 are processed. Allocates internal RIP state.
 *
 * @param node Router node to initialise RIP on.
 * @return MAGI_OK on success, otherwise an error code.
 */
int rip_init(Node* node);

/**
 * @brief Send a RIP update to all neighbouring routers.
 *
 * Iterates all interfaces on the router. For each interface that has
 * both an IPv4 address and a connected link, builds a RIP response
 * message containing all routing table entries with metric < INFINITY,
 * wraps it in a UDP/IPv4 packet, and sends it to the neighbour.
 *
 * @param node Router node to send the update from.
 * @return MAGI_OK on success, otherwise an error code.
 */
int rip_send_update(Node* node);

/**
 * @brief Return whether RIP is active on a node.
 *
 * @param node Node to inspect.
 * @return true when the node has active RIP state.
 */
bool rip_is_active(const Node* node);

/**
 * @brief Remove RIP-learned routes that used a disconnected port.
 *
 * Called by the CLI after unlinking a cable so stale dynamic routes do
 * not continue to win LPM while the next triggered update converges.
 *
 * @param node Router node.
 * @param port Disconnected local port number.
 * @return MAGI_OK on success, otherwise an error code.
 */
int rip_handle_link_down(Node* node, uint16_t port);

/**
 * @brief Process a received RIP message (Bellman-Ford update).
 *
 * Parses the RIP response entries. For each entry, if the network
 * matches a known route and the metric via the sender + 1 is lower
 * than the current metric (and < INFINITY), the route is updated
 * (next_hop = sender, metric = via + 1, out_port = egress to sender).
 * If any route changes, a triggered update is sent.
 *
 * @param node      Router node that received the message.
 * @param data      RIP message payload (after UDP header).
 * @param len       Length of the message in bytes.
 * @param sender_ip IPv4 address of the sending router.
 */
void rip_handle_message(Node* node, const uint8_t* data, size_t len, const uint8_t sender_ip[4]);

#endif /* MAGI_LAYER7_RIP_H */
