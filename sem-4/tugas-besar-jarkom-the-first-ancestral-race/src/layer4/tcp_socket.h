/**
 * @file tcp_socket.h
 * @brief TCP socket state machine and receive buffering.
 */

#ifndef MAGI_LAYER4_TCP_SOCKET_H
#define MAGI_LAYER4_TCP_SOCKET_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "core/node.h"
#include "layer4/tcp.h"
#include "utils/hashmap.h"

/* ─── TCP states ─── */
typedef enum TCPState {
  TCP_CLOSED,
  TCP_LISTEN,
  TCP_SYN_SENT,
  TCP_SYN_RCVD,
  TCP_ESTABLISHED,
  TCP_FIN_WAIT_1,
  TCP_FIN_WAIT_2,
  TCP_CLOSE_WAIT,
  TCP_LAST_ACK,
  TCP_TIME_WAIT,
  TCP_CLOSING
} TCPState;

/** @brief State name strings for logging. */
extern const char* tcp_state_name(TCPState state);

/* ─── Out-of-order segment list ─── */
typedef struct OOOSegment OOOSegment;
struct OOOSegment {
  uint32_t seq_num;
  uint8_t* data;
  size_t len;
  OOOSegment* next;
};

/* ─── TCP socket ─── */
typedef struct TCPSocket {
  TCPState state;
  uint8_t local_ip[4];
  uint16_t local_port;
  uint8_t remote_ip[4];
  uint16_t remote_port;
  uint32_t seq_num; /* next seq to send */
  uint32_t ack_num; /* next expected seq */
  uint8_t* recv_buf;
  size_t recv_buf_len;
  size_t recv_buf_cap;
  OOOSegment* out_of_order;
  struct Node* node;
  bool active_open; /* true = we initiated the connection */
} TCPSocket;

/**
 * @brief Allocate and initialise a TCP socket (state = CLOSED).
 *
 * recv_buf is allocated with 16 KB capacity.
 *
 * @param node Owning node.
 * @return New socket, or NULL on failure.
 */
TCPSocket* tcp_socket_new(struct Node* node);

/**
 * @brief Destroy a TCP socket and free all resources.
 *
 * @param sock Socket to free. NULL is allowed.
 */
void tcp_socket_free(TCPSocket* sock);

/**
 * @brief Process an incoming TCP segment through the state machine.
 *
 * Implements the exhaustive transition table from AGENTS.md §11.2.
 * Sends response segments via node->send_ip_packet.
 *
 * @param sock Target socket.
 * @param seg  Parsed incoming segment.
 * @param node Owning node (for sending responses).
 * @return MAGI_OK on success, otherwise an error code.
 */
int tcp_socket_handle_segment(TCPSocket* sock, TCPSegment* seg, struct Node* node,
                              const uint8_t src_ip[4], const uint8_t dst_ip[4]);

/**
 * @brief Read contiguous data from the receive buffer.
 *
 * Copies up to len bytes from recv_buf and shifts remaining data.
 *
 * @param sock Socket with data.
 * @param out  Output buffer.
 * @param len  Requested bytes.
 * @return Number of bytes actually read.
 */
size_t tcp_recv_buf_read(TCPSocket* sock, uint8_t* out, size_t len);

/**
 * @brief Check if the socket has data available.
 *
 * @param sock TCP socket.
 * @return true if recv_buf_len > 0.
 */
bool tcp_socket_has_data(const TCPSocket* sock);

/**
 * @brief Send a bare RST segment (used for closed-port rejection).
 *
 * @param node    Node to send from.
 * @param src_ip  Source IP.
 * @param dst_ip  Destination IP.
 * @param src_port Source port.
 * @param dst_port Destination port.
 * @param seq_num  Sequence number for RST.
 * @param ack_num  Ack number for RST.
 */
void tcp_send_rst_packet(struct Node* node, const uint8_t src_ip[4], const uint8_t dst_ip[4],
                         uint16_t src_port, uint16_t dst_port, uint32_t seq_num, uint32_t ack_num);

/* ─── High-level API ─── */

/**
 * @brief Bind a TCP socket to a local address and port.
 *
 * The socket transitions to LISTEN state.
 * The socket's local_ip and local_port are set from the node's first interface.
 *
 * @param sock  TCP socket.
 * @param node  Owning node.
 * @param port  Local port to bind.
 * @return MAGI_OK on success, MAGI_ERR_PORTUSED if port is already bound.
 */
int tcp_socket_bind_listen(TCPSocket* sock, struct Node* node, uint16_t port);

/**
 * @brief Initiate an active open (client connection).
 *
 * The socket is placed in SYN_SENT state and a SYN is sent.
 * On success, the full 3-way handshake completes within this call
 * (synchronous sequential mode).
 *
 * @param sock    TCP socket (must be in CLOSED state).
 * @param node    Owning node.
 * @param dst_ip  Destination IPv4 address (dotted decimal).
 * @param dst_port Destination port.
 * @return MAGI_OK on success, MAGI_ERR_CONNRESET if handshake fails.
 */
int tcp_socket_connect(TCPSocket* sock, struct Node* node, const char* dst_ip, uint16_t dst_port);

/**
 * @brief Send data on an ESTABLISHED socket.
 *
 * @param sock  TCP socket (must be ESTABLISHED).
 * @param node  Owning node.
 * @param data  Payload bytes.
 * @param len   Payload length.
 * @return MAGI_OK on success.
 */
int tcp_socket_send(TCPSocket* sock, struct Node* node, const uint8_t* data, size_t len);

/**
 * @brief Initiate active close (send FIN).
 *
 * The socket transitions from ESTABLISHED to FIN_WAIT_1.
 *
 * @param sock  TCP socket.
 * @param node  Owning node.
 * @return MAGI_OK on success.
 */
int tcp_socket_close(TCPSocket* sock, struct Node* node);

#endif /* MAGI_LAYER4_TCP_SOCKET_H */
