#define _POSIX_C_SOURCE 200809L

#include "udp_socket.h"

#include "utils/magi_error.h"

#include <stdlib.h>
#include <string.h>

UDPSocketState* udp_socket_new(struct Node* node) {
  UDPSocketState* sock = calloc(1U, sizeof(*sock));
  if (sock == NULL) {
    magi_errno = MAGI_ERR_NOMEM;
    return NULL;
  }

  sock->recv_buf_cap = UDP_SOCKET_RECV_BUF_CAP;
  sock->recv_buf = malloc(sock->recv_buf_cap);
  if (sock->recv_buf == NULL) {
    free(sock);
    magi_errno = MAGI_ERR_NOMEM;
    return NULL;
  }

  sock->recv_buf_len = 0U;
  sock->node = node;
  return sock;
}

void udp_socket_free(UDPSocketState* sock) {
  if (sock == NULL) {
    return;
  }
  free(sock->recv_buf);
  free(sock);
}

int udp_socket_deliver(UDPSocketState* sock, const uint8_t src_ip[4], uint16_t src_port,
                       const uint8_t* payload, size_t payload_len) {
  if (sock == NULL || payload == NULL) {
    magi_errno = MAGI_ERR_BADARGS;
    return MAGI_ERR_BADARGS;
  }

  /* Overwrite buffer with new datagram (one-at-a-time semantics). */
  if (payload_len > sock->recv_buf_cap) {
    payload_len = sock->recv_buf_cap;
  }

  memcpy(sock->recv_buf, payload, payload_len);
  sock->recv_buf_len = payload_len;
  memcpy(sock->last_src_ip, src_ip, 4U);
  sock->last_src_port = src_port;
  return MAGI_OK;
}

size_t udp_socket_read(UDPSocketState* sock, uint8_t* out, size_t len) {
  if (sock == NULL || out == NULL || len == 0U) {
    return 0U;
  }

  size_t to_copy = sock->recv_buf_len < len ? sock->recv_buf_len : len;
  if (to_copy == 0U) {
    return 0U;
  }

  memcpy(out, sock->recv_buf, to_copy);
  size_t remaining = sock->recv_buf_len - to_copy;
  if (remaining > 0U) {
    memmove(sock->recv_buf, sock->recv_buf + to_copy, remaining);
  }
  sock->recv_buf_len = remaining;
  return to_copy;
}

bool udp_socket_has_data(const UDPSocketState* sock) {
  return sock != NULL && sock->recv_buf_len > 0U;
}
