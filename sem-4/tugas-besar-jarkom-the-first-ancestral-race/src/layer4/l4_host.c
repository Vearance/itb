#define _POSIX_C_SOURCE 200809L

#include "l4_host.h"

#include "layer3/ipv4.h"
#include "layer4/port_registry.h"
#include "layer4/tcp.h"
#include "layer4/tcp_socket.h"
#include "layer4/udp.h"
#include "layer4/udp_socket.h"
#include "utils/log.h"
#include "utils/magi_error.h"

#include <stdlib.h>
#include <string.h>

/* ─── Forward declarations ─── */

static void l4_dispatch_packet(struct Node* node, const uint8_t src_ip[4], const uint8_t dst_ip[4],
                               uint8_t protocol, const uint8_t* payload, size_t payload_len);

static void l4_data_destroy(void* data);

/* ─── Public API ─── */

/**
 * @brief Attach L4 (TCP/UDP) support to a host node.
 *
 * Creates a port registry HashMap, stores it in node->l4_data, and
 * registers the L4 dispatch and IP send callback function pointers.
 * If the node already has L4 data attached, only the callbacks are
 * updated and the existing registry is reused.
 *
 * @param node  The host node to attach L4 to.
 * @return MAGI_OK on success, MAGI_ERR_NOMEM on allocation failure,
 *         MAGI_ERR_BADARGS if node is NULL.
 */
int l4_host_attach(Node* node) {
  if (node == NULL) {
    magi_errno = MAGI_ERR_BADARGS;
    return MAGI_ERR_BADARGS;
  }

  /* If already attached, just update callbacks */
  if (node->l4_data != NULL) {
    node->handle_l4_packet = l4_dispatch_packet;
    node->send_ip_packet = ipv4_send_packet;
    return MAGI_OK;
  }

  HashMap* reg = port_registry_new();
  if (reg == NULL) {
    return MAGI_ERR_NOMEM;
  }

  node->l4_data = reg;
  node->l4_data_free = l4_data_destroy;
  node->handle_l4_packet = l4_dispatch_packet;
  node->send_ip_packet = ipv4_send_packet;
  return MAGI_OK;
}

/**
 * @brief Retrieve the port registry from a node's L4 data.
 *
 * @param node  The node whose port registry to retrieve.
 * @return HashMappointer (the port registry), or NULL if node is NULL
 *         or L4 has not been attached.
 */
void* l4_host_get_registry(Node* node) {
  return (node != NULL) ? node->l4_data : NULL;
}

/* ─── Internal helpers ─── */

/**
 * @brief Free all L4 data associated with a node.
 *
 * Iterates over the port registry, frees every PortBinding via
 * port_registry_free_binding, then frees the hashmap itself.
 *
 * @param data  Opaque pointer to the port registry HashMap.
 */
static void l4_data_destroy(void* data) {
  if (data == NULL) {
    return;
  }
  HashMap* reg = (HashMap*)data;
  hashmap_foreach(reg, port_registry_free_binding, NULL);
  hashmap_free(reg);
}

/**
 * @brief Dispatch an L4 packet to the appropriate protocol handler.
 *
 * Called via node->handle_l4_packet when an IPv4 packet with a
 * transport protocol (TCP or UDP) arrives. Parses the transport
 * header, looks up the bound socket in the port registry, and
 * either delivers the segment to the socket state machine (TCP)
 * or logs the datagram (UDP). Unbound TCP ports receive a RST.
 *
 * @param node         Receiving node.
 * @param src_ip       Source IPv4 address (4 bytes).
 * @param dst_ip       Destination IPv4 address (4 bytes).
 * @param protocol     L4 protocol number (6 = TCP, 17 = UDP).
 * @param payload      Pointer to the transport header+data.
 * @param payload_len  Length of the transport payload.
 */
static void l4_dispatch_packet(struct Node* node, const uint8_t src_ip[4], const uint8_t dst_ip[4],
                               uint8_t protocol, const uint8_t* payload, size_t payload_len) {
  if (node == NULL || src_ip == NULL || dst_ip == NULL || payload == NULL) {
    return;
  }

  HashMap* reg = (HashMap*)node->l4_data;
  if (reg == NULL) {
    LOG(node->name, "L4 not attached: drop protocol %u", (unsigned)protocol);
    return;
  }

  switch (protocol) {

  case IPV4_PROTOCOL_TCP: {
    /* Parse TCP segment */
    TCPSegment seg;
    memset(&seg, 0, sizeof(seg));
    if (tcp_unpack(&seg, src_ip, dst_ip, payload, payload_len) != MAGI_OK) {
      LOG(node->name, "Drop TCP segment: bad checksum");
      return;
    }

    /* Look up bound socket */
    void* sock = port_registry_lookup(reg, PORT_PROTOCOL_TCP, seg.dst_port);
    if (sock == NULL) {
      LOG(node->name, "TCP port %u not bound; send RST", (unsigned)seg.dst_port);
      /* Send RST for unbound port */
      tcp_send_rst_packet(node, dst_ip, src_ip, seg.dst_port, seg.src_port, seg.ack_num,
                          seg.seq_num + (uint32_t)(seg.payload_len > 0U ? seg.payload_len : 1U));
      return;
    }

    /* Let the socket state machine handle it */
    (void)tcp_socket_handle_segment((TCPSocket*)sock, &seg, node, src_ip, dst_ip);
    return;
  }

  case IPV4_PROTOCOL_UDP: {
    /* Parse UDP datagram */
    UDPDatagram dgram;
    memset(&dgram, 0, sizeof(dgram));
    if (udp_unpack(&dgram, src_ip, dst_ip, payload, payload_len) != MAGI_OK) {
      LOG(node->name, "Drop UDP datagram: bad checksum");
      return;
    }

    /* Look up bound socket */
    void* sock = port_registry_lookup(reg, PORT_PROTOCOL_UDP, dgram.dst_port);
    if (sock == NULL) {
      LOG(node->name, "UDP port %u not bound; drop silently", (unsigned)dgram.dst_port);
      return;
    }

    /* Deliver payload into UDPSocketState receive buffer */
    UDPSocketState* udp_sock = (UDPSocketState*)sock;
    (void)udp_socket_deliver(udp_sock, src_ip, dgram.src_port, dgram.payload, dgram.payload_len);
    LOG(node->name, "UDP datagram received on port %u (%zu bytes)", (unsigned)dgram.dst_port,
        dgram.payload_len);
    return;
  }

  default:
    LOG(node->name, "Unsupported L4 protocol %u", (unsigned)protocol);
    return;
  }
}
