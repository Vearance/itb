#define _POSIX_C_SOURCE 200809L

#include "magi_socket.h"

#include "core/node.h"
#include "layer3/ipv4.h"
#include "layer4/l4_host.h"
#include "layer4/port_registry.h"
#include "layer4/tcp.h"
#include "layer4/tcp_socket.h"
#include "layer4/udp.h"
#include "layer4/udp_socket.h"
#include "utils/log.h"
#include "utils/magi_error.h"

#include <stdlib.h>
#include <string.h>

MagiSocket* magi_socket(struct Node* node, MagiAddrFamily family, MagiSockType type) {
  if (node == NULL || family != MAGI_AF_INET) {
    magi_errno = MAGI_ERR_BADARGS;
    return NULL;
  }

  MagiSocket* sock = calloc(1U, sizeof(*sock));
  if (sock == NULL) {
    magi_errno = MAGI_ERR_NOMEM;
    return NULL;
  }

  sock->family = family;
  sock->type = type;
  sock->node = node;
  sock->bound = false;
  sock->listening = false;
  sock->local_port = 0U;

  if (type == MAGI_SOCK_STREAM) {
    TCPSocket* tcp = tcp_socket_new(node);
    if (tcp == NULL) {
      free(sock);
      return NULL;
    }
    sock->transport = tcp;
  } else if (type == MAGI_SOCK_DGRAM) {
    UDPSocketState* udp = udp_socket_new(node);
    if (udp == NULL) {
      free(sock);
      return NULL;
    }
    sock->transport = udp;
  } else {
    free(sock);
    magi_errno = MAGI_ERR_BADARGS;
    return NULL;
  }

  return sock;
}

int magi_bind(MagiSocket* sock, const char* ip, uint16_t port) {
  if (sock == NULL || ip == NULL || port == 0U) {
    magi_errno = MAGI_ERR_BADARGS;
    return MAGI_ERR_BADARGS;
  }

  HashMap* reg = (HashMap*)l4_host_get_registry(sock->node);
  if (reg == NULL) {
    LOG(sock->node->name, "magi_bind: L4 not attached");
    magi_errno = MAGI_ERR_BADARGS;
    return MAGI_ERR_BADARGS;
  }

  uint8_t ip_bytes[4];
  if (ipv4_parse_address(ip, ip_bytes) != MAGI_OK) {
    magi_errno = MAGI_ERR_BADARGS;
    return MAGI_ERR_BADARGS;
  }

  if (sock->type == MAGI_SOCK_STREAM) {
    TCPSocket* tcp = (TCPSocket*)sock->transport;
    memcpy(tcp->local_ip, ip_bytes, 4U);
    tcp->local_port = port;
  } else {
    UDPSocketState* udp = (UDPSocketState*)sock->transport;
    memcpy(udp->local_ip, ip_bytes, 4U);
    udp->local_port = port;
  }

  uint8_t proto = (sock->type == MAGI_SOCK_STREAM) ? PORT_PROTOCOL_TCP : PORT_PROTOCOL_UDP;
  int status = port_registry_bind(reg, proto, port, sock->transport);
  if (status != MAGI_OK) {
    return status;
  }

  sock->local_port = port;
  sock->bound = true;
  LOG(sock->node->name, "magi_bind: bound %s port %u",
      sock->type == MAGI_SOCK_STREAM ? "TCP" : "UDP", (unsigned)port);
  return MAGI_OK;
}

int magi_listen(MagiSocket* sock, int backlog) {
  (void)backlog; /* ignored in sequential mode */

  if (sock == NULL || sock->type != MAGI_SOCK_STREAM) {
    magi_errno = MAGI_ERR_BADARGS;
    return MAGI_ERR_BADARGS;
  }

  if (!sock->bound) {
    LOG(sock->node->name, "magi_listen: socket not bound");
    magi_errno = MAGI_ERR_BADARGS;
    return MAGI_ERR_BADARGS;
  }

  TCPSocket* tcp = (TCPSocket*)sock->transport;
  tcp->state = TCP_LISTEN;
  sock->listening = true;
  LOG(sock->node->name, "magi_listen: TCP LISTEN on port %u", (unsigned)sock->local_port);
  return MAGI_OK;
}

MagiSocket* magi_accept(MagiSocket* sock) {
  if (sock == NULL || sock->type != MAGI_SOCK_STREAM || !sock->listening) {
    magi_errno = MAGI_ERR_BADARGS;
    return NULL;
  }

  TCPSocket* listen_tcp = (TCPSocket*)sock->transport;

  /* In sequential mode the handshake already completed synchronously
     when the remote host's SYN arrived via l4_dispatch_packet and was
     processed by tcp_socket_handle_segment on the LISTEN socket.
     The listening socket transitions LISTEN → SYN_RCVD → ESTABLISHED.
     We create a new MagiSocket wrapping the now-ESTABLISHED socket
     and create a fresh listening socket to take its place. */

  if (listen_tcp->state != TCP_ESTABLISHED && listen_tcp->state != TCP_SYN_RCVD) {
    LOG(sock->node->name, "magi_accept: no pending connection (state=%s)",
        tcp_state_name(listen_tcp->state));
    magi_errno = MAGI_ERR_BADARGS;
    return NULL;
  }

  /* Create the accepted connection socket */
  MagiSocket* accepted = calloc(1U, sizeof(*accepted));
  if (accepted == NULL) {
    magi_errno = MAGI_ERR_NOMEM;
    return NULL;
  }

  accepted->family = MAGI_AF_INET;
  accepted->type = MAGI_SOCK_STREAM;
  accepted->node = sock->node;
  accepted->transport = listen_tcp;
  accepted->bound = true;
  accepted->listening = false;
  accepted->local_port = sock->local_port;

  /* Create a new listening socket to replace the one we just consumed */
  TCPSocket* new_listen = tcp_socket_new(sock->node);
  if (new_listen == NULL) {
    free(accepted);
    magi_errno = MAGI_ERR_NOMEM;
    return NULL;
  }

  memcpy(new_listen->local_ip, listen_tcp->local_ip, 4U);
  new_listen->local_port = listen_tcp->local_port;
  new_listen->state = TCP_LISTEN;

  /* Update port registry to point to the new listening socket */
  HashMap* reg = (HashMap*)l4_host_get_registry(sock->node);
  if (reg != NULL) {
    (void)port_registry_unbind(reg, PORT_PROTOCOL_TCP, sock->local_port);
    (void)port_registry_bind(reg, PORT_PROTOCOL_TCP, sock->local_port, new_listen);
  }

  sock->transport = new_listen;
  return accepted;
}

int magi_connect(MagiSocket* sock, const char* ip, uint16_t port) {
  if (sock == NULL || ip == NULL || port == 0U) {
    magi_errno = MAGI_ERR_BADARGS;
    return MAGI_ERR_BADARGS;
  }

  if (sock->type == MAGI_SOCK_STREAM) {
    TCPSocket* tcp = (TCPSocket*)sock->transport;

    /* Auto-bind to ephemeral port if not already bound */
    if (!sock->bound) {
      HashMap* reg = (HashMap*)l4_host_get_registry(sock->node);
      if (reg == NULL) {
        magi_errno = MAGI_ERR_BADARGS;
        return MAGI_ERR_BADARGS;
      }
      uint16_t ephemeral = (uint16_t)(49152U + ((uintptr_t)tcp & 0x3FFFU));
      int status = port_registry_bind(reg, PORT_PROTOCOL_TCP, ephemeral, tcp);
      if (status != MAGI_OK) {
        return status;
      }
      sock->local_port = ephemeral;
      sock->bound = true;
    }

    int status = tcp_socket_connect(tcp, sock->node, ip, port);
    if (status != MAGI_OK) {
      return status;
    }

    LOG(sock->node->name, "magi_connect: TCP connected to %s:%u (state=%s)", ip, (unsigned)port,
        tcp_state_name(tcp->state));
    return MAGI_OK;
  }

  if (sock->type == MAGI_SOCK_DGRAM) {
    UDPSocketState* udp = (UDPSocketState*)sock->transport;
    uint8_t dst_bytes[4];
    if (ipv4_parse_address(ip, dst_bytes) != MAGI_OK) {
      magi_errno = MAGI_ERR_BADARGS;
      return MAGI_ERR_BADARGS;
    }
    memcpy(udp->remote_ip, dst_bytes, 4U);
    udp->remote_port = port;
    return MAGI_OK;
  }

  magi_errno = MAGI_ERR_BADARGS;
  return MAGI_ERR_BADARGS;
}

int magi_send(MagiSocket* sock, const uint8_t* data, size_t len) {
  if (sock == NULL || (len > 0U && data == NULL)) {
    magi_errno = MAGI_ERR_BADARGS;
    return MAGI_ERR_BADARGS;
  }

  if (sock->type == MAGI_SOCK_STREAM) {
    TCPSocket* tcp = (TCPSocket*)sock->transport;
    return tcp_socket_send(tcp, sock->node, data, len);
  }

  if (sock->type == MAGI_SOCK_DGRAM) {
    UDPSocketState* udp = (UDPSocketState*)sock->transport;

    /* Build and send the UDP datagram via node->send_ip_packet */
    UDPDatagram dgram;
    memset(&dgram, 0, sizeof(dgram));
    dgram.src_port = udp->local_port;
    dgram.dst_port = udp->remote_port;
    dgram.payload = data;
    dgram.payload_len = len;

    size_t total = UDP_HEADER_LEN + len;
    uint8_t* buf = malloc(total);
    if (buf == NULL) {
      magi_errno = MAGI_ERR_NOMEM;
      return MAGI_ERR_NOMEM;
    }

    int status = udp_pack(&dgram, udp->local_ip, udp->remote_ip, buf, total);
    if (status != MAGI_OK) {
      free(buf);
      return status;
    }

    if (sock->node->send_ip_packet == NULL) {
      free(buf);
      magi_errno = MAGI_ERR_BADARGS;
      return MAGI_ERR_BADARGS;
    }

    status = sock->node->send_ip_packet(sock->node, udp->local_ip, udp->remote_ip,
                                        IPV4_PROTOCOL_UDP, IPV4_DEFAULT_TTL, buf, total);
    free(buf);
    return status;
  }

  magi_errno = MAGI_ERR_BADARGS;
  return MAGI_ERR_BADARGS;
}

int magi_sendto(MagiSocket* sock, const uint8_t* data, size_t len, const char* dst_ip,
                uint16_t dst_port) {
  if (sock == NULL || dst_ip == NULL || dst_port == 0U) {
    magi_errno = MAGI_ERR_BADARGS;
    return MAGI_ERR_BADARGS;
  }

  if (sock->type != MAGI_SOCK_DGRAM) {
    magi_errno = MAGI_ERR_BADARGS;
    return MAGI_ERR_BADARGS;
  }

  UDPSocketState* udp = (UDPSocketState*)sock->transport;

  uint8_t dst_bytes[4];
  if (ipv4_parse_address(dst_ip, dst_bytes) != MAGI_OK) {
    magi_errno = MAGI_ERR_BADARGS;
    return MAGI_ERR_BADARGS;
  }

  UDPDatagram dgram;
  memset(&dgram, 0, sizeof(dgram));
  dgram.src_port = udp->local_port;
  dgram.dst_port = dst_port;
  dgram.payload = data;
  dgram.payload_len = len;

  size_t total = UDP_HEADER_LEN + len;
  uint8_t* buf = malloc(total);
  if (buf == NULL) {
    magi_errno = MAGI_ERR_NOMEM;
    return MAGI_ERR_NOMEM;
  }

  int status = udp_pack(&dgram, udp->local_ip, dst_bytes, buf, total);
  if (status != MAGI_OK) {
    free(buf);
    return status;
  }

  if (sock->node->send_ip_packet == NULL) {
    free(buf);
    magi_errno = MAGI_ERR_BADARGS;
    return MAGI_ERR_BADARGS;
  }

  status = sock->node->send_ip_packet(sock->node, udp->local_ip, dst_bytes, IPV4_PROTOCOL_UDP,
                                      IPV4_DEFAULT_TTL, buf, total);
  free(buf);
  return status;
}

int magi_recv(MagiSocket* sock, uint8_t* buf, size_t buf_len) {
  if (sock == NULL || buf == NULL || buf_len == 0U) {
    magi_errno = MAGI_ERR_BADARGS;
    return MAGI_ERR_BADARGS;
  }

  if (sock->type == MAGI_SOCK_STREAM) {
    TCPSocket* tcp = (TCPSocket*)sock->transport;
    size_t rd = tcp_recv_buf_read(tcp, buf, buf_len);
    return (int)rd;
  }

  if (sock->type == MAGI_SOCK_DGRAM) {
    UDPSocketState* udp = (UDPSocketState*)sock->transport;
    size_t rd = udp_socket_read(udp, buf, buf_len);
    return (int)rd;
  }

  magi_errno = MAGI_ERR_BADARGS;
  return MAGI_ERR_BADARGS;
}

int magi_recvfrom(MagiSocket* sock, uint8_t* buf, size_t buf_len, char* src_ip_out,
                  uint16_t* src_port_out) {
  if (sock == NULL || buf == NULL || buf_len == 0U) {
    magi_errno = MAGI_ERR_BADARGS;
    return MAGI_ERR_BADARGS;
  }

  if (sock->type != MAGI_SOCK_DGRAM) {
    magi_errno = MAGI_ERR_BADARGS;
    return MAGI_ERR_BADARGS;
  }

  UDPSocketState* udp = (UDPSocketState*)sock->transport;

  /* Capture sender info before reading */
  if (src_ip_out != NULL) {
    ipv4_address_to_string(udp->last_src_ip, src_ip_out);
  }
  if (src_port_out != NULL) {
    *src_port_out = udp->last_src_port;
  }

  size_t rd = udp_socket_read(udp, buf, buf_len);
  return (int)rd;
}

bool magi_has_data(MagiSocket* sock) {
  if (sock == NULL) {
    return false;
  }

  if (sock->type == MAGI_SOCK_STREAM) {
    TCPSocket* tcp = (TCPSocket*)sock->transport;
    return tcp_socket_has_data(tcp);
  }

  if (sock->type == MAGI_SOCK_DGRAM) {
    UDPSocketState* udp = (UDPSocketState*)sock->transport;
    return udp_socket_has_data(udp);
  }

  return false;
}

int magi_close(MagiSocket* sock) {
  if (sock == NULL) {
    return MAGI_OK;
  }

  HashMap* reg = (HashMap*)l4_host_get_registry(sock->node);

  if (sock->type == MAGI_SOCK_STREAM) {
    TCPSocket* tcp = (TCPSocket*)sock->transport;
    if (tcp != NULL) {
      if (tcp->state == TCP_ESTABLISHED || tcp->state == TCP_CLOSE_WAIT) {
        (void)tcp_socket_close(tcp, sock->node);
      }
      /* Unbind from port registry */
      if (reg != NULL && sock->bound) {
        (void)port_registry_unbind(reg, PORT_PROTOCOL_TCP, sock->local_port);
      }
      tcp_socket_free(tcp);
    }
  } else if (sock->type == MAGI_SOCK_DGRAM) {
    UDPSocketState* udp = (UDPSocketState*)sock->transport;
    if (udp != NULL) {
      if (reg != NULL && sock->bound) {
        (void)port_registry_unbind(reg, PORT_PROTOCOL_UDP, sock->local_port);
      }
      udp_socket_free(udp);
    }
  }

  LOG(sock->node->name, "magi_close: socket closed (port %u)", (unsigned)sock->local_port);
  free(sock);
  return MAGI_OK;
}
