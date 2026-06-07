/**
 * @file udp_socket.h
 * @brief Stateful UDP socket wrapper for the MagiSocket API.
 *
 * Provides a receive buffer so that application-layer protocols (DHCP, DNS)
 * can receive UDP datagrams via magi_recv().
 */

#ifndef MAGI_LAYER4_UDP_SOCKET_H
#define MAGI_LAYER4_UDP_SOCKET_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

struct Node;

#define UDP_SOCKET_RECV_BUF_CAP 4096U

/**
 * @brief Stateful UDP socket holding local/remote address and receive buffer.
 */
typedef struct UDPSocketState {
  uint8_t local_ip[4];
  uint16_t local_port;
  uint8_t remote_ip[4];
  uint16_t remote_port;
  uint8_t* recv_buf;
  size_t recv_buf_len;
  size_t recv_buf_cap;
  /** Source IP of the most recently received datagram. */
  uint8_t last_src_ip[4];
  /** Source port of the most recently received datagram. */
  uint16_t last_src_port;
  struct Node* node;
} UDPSocketState;

/**
 * @brief Allocate and initialise a UDP socket state.
 *
 * @param node Owning node.
 * @return New socket, or NULL on failure.
 */
UDPSocketState* udp_socket_new(struct Node* node);

/**
 * @brief Destroy a UDP socket state and free all resources.
 *
 * @param sock Socket to free. NULL is allowed.
 */
void udp_socket_free(UDPSocketState* sock);

/**
 * @brief Deliver a received datagram payload into the socket's buffer.
 *
 * Called by the L4 dispatch when a UDP datagram arrives on this socket's port.
 *
 * @param sock       Target UDP socket.
 * @param src_ip     Source IPv4 address (4 bytes).
 * @param src_port   Source port.
 * @param payload    Datagram payload bytes.
 * @param payload_len Payload length.
 * @return 0 on success, negative on error.
 */
int udp_socket_deliver(UDPSocketState* sock, const uint8_t src_ip[4], uint16_t src_port,
                       const uint8_t* payload, size_t payload_len);

/**
 * @brief Read data from the UDP socket's receive buffer.
 *
 * Copies up to len bytes and shifts remaining data.
 *
 * @param sock Socket to read from.
 * @param out  Output buffer.
 * @param len  Maximum bytes to read.
 * @return Number of bytes actually read.
 */
size_t udp_socket_read(UDPSocketState* sock, uint8_t* out, size_t len);

/**
 * @brief Check if the UDP socket has data available.
 *
 * @param sock UDP socket.
 * @return true if recv_buf_len > 0.
 */
bool udp_socket_has_data(const UDPSocketState* sock);

#endif /* MAGI_LAYER4_UDP_SOCKET_H */
