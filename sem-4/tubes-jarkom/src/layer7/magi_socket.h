/**
 * @file magi_socket.h
 * @brief MagiSocket wrapper API — BSD-like socket interface for the simulator.
 *
 * All application protocols (DHCP, DNS, HTTP) use only this API.
 * They never call tcp_pack, udp_pack, or any layer4 function directly.
 */

#ifndef MAGI_LAYER7_MAGI_SOCKET_H
#define MAGI_LAYER7_MAGI_SOCKET_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

struct Node;

typedef enum { MAGI_AF_INET = 2 } MagiAddrFamily;
typedef enum { MAGI_SOCK_STREAM = 1, MAGI_SOCK_DGRAM = 2 } MagiSockType;

typedef struct MagiSocket {
  MagiAddrFamily family;
  MagiSockType type;
  struct Node* node;
  void* transport; /* TCPSocket* or UDPSocketState* */
  bool bound;
  bool listening;
  uint16_t local_port;
} MagiSocket;

/**
 * @brief Create a new MagiSocket.
 *
 * @param node   Owning node.
 * @param family Address family (must be MAGI_AF_INET).
 * @param type   Socket type (MAGI_SOCK_STREAM or MAGI_SOCK_DGRAM).
 * @return New socket, or NULL on failure.
 */
MagiSocket* magi_socket(struct Node* node, MagiAddrFamily family, MagiSockType type);

/**
 * @brief Bind a socket to a local address and port.
 *
 * @param sock Socket to bind.
 * @param ip   Local IP address (dotted decimal string).
 * @param port Local port number.
 * @return MAGI_OK on success, otherwise an error code.
 */
int magi_bind(MagiSocket* sock, const char* ip, uint16_t port);

/**
 * @brief Set a STREAM socket to listening state.
 *
 * @param sock    TCP socket to listen on.
 * @param backlog Connection backlog (ignored in sequential mode).
 * @return MAGI_OK on success, otherwise an error code.
 */
int magi_listen(MagiSocket* sock, int backlog);

/**
 * @brief Accept an incoming connection on a listening STREAM socket.
 *
 * In sequential mode, the handshake completes synchronously before returning.
 *
 * @param sock Listening socket.
 * @return New connected MagiSocket, or NULL on failure.
 */
MagiSocket* magi_accept(MagiSocket* sock);

/**
 * @brief Connect a socket to a remote address.
 *
 * For STREAM sockets, completes the 3-way handshake before returning.
 * For DGRAM sockets, just sets the remote address.
 *
 * @param sock Socket to connect.
 * @param ip   Remote IP address (dotted decimal string).
 * @param port Remote port number.
 * @return MAGI_OK on success, otherwise an error code.
 */
int magi_connect(MagiSocket* sock, const char* ip, uint16_t port);

/**
 * @brief Send data on a connected socket.
 *
 * @param sock Socket to send on.
 * @param data Payload bytes.
 * @param len  Payload length.
 * @return MAGI_OK on success, otherwise an error code.
 */
int magi_send(MagiSocket* sock, const uint8_t* data, size_t len);

/**
 * @brief Send a UDP datagram to a specific destination.
 *
 * @param sock    DGRAM socket to send from.
 * @param data    Payload bytes.
 * @param len     Payload length.
 * @param dst_ip  Destination IP (dotted decimal string).
 * @param dst_port Destination port.
 * @return MAGI_OK on success, otherwise an error code.
 */
int magi_sendto(MagiSocket* sock, const uint8_t* data, size_t len, const char* dst_ip,
                uint16_t dst_port);

/**
 * @brief Receive data from a socket.
 *
 * @param sock    Socket to receive from.
 * @param buf     Output buffer.
 * @param buf_len Output buffer capacity.
 * @return Number of bytes received (cast to int), or negative error code.
 */
int magi_recv(MagiSocket* sock, uint8_t* buf, size_t buf_len);

/**
 * @brief Receive data from a DGRAM socket and get the sender's address.
 *
 * @param sock       DGRAM socket to receive from.
 * @param buf        Output buffer.
 * @param buf_len    Output buffer capacity.
 * @param src_ip_out Output buffer for sender IP (16 bytes, dotted decimal).
 * @param src_port_out Output for sender port.
 * @return Number of bytes received (cast to int), or negative error code.
 */
int magi_recvfrom(MagiSocket* sock, uint8_t* buf, size_t buf_len, char* src_ip_out,
                  uint16_t* src_port_out);

/**
 * @brief Check if a socket has data available to read.
 *
 * @param sock Socket to check.
 * @return true if there is data available.
 */
bool magi_has_data(MagiSocket* sock);

/**
 * @brief Close a socket and release all resources.
 *
 * For STREAM sockets, initiates the 4-way teardown.
 *
 * @param sock Socket to close.
 * @return MAGI_OK on success, otherwise an error code.
 */
int magi_close(MagiSocket* sock);

#endif /* MAGI_LAYER7_MAGI_SOCKET_H */
