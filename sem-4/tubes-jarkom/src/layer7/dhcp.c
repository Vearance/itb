#define _POSIX_C_SOURCE 200809L

#include "dhcp.h"

#include "core/interface.h"
#include "layer3/ipv4.h"
#include "layer7/magi_socket.h"
#include "utils/byteops.h"
#include "utils/log.h"
#include "utils/magi_error.h"

#include <stdlib.h>
#include <string.h>

/*
 * Simplified DHCP message layout (minimum 240 bytes + options):
 *   op[1] htype[1] hlen[1] hops[1]
 *   xid[4]
 *   secs[2] flags[2]
 *   ciaddr[4] yiaddr[4] siaddr[4] giaddr[4]
 *   chaddr[16]
 *   sname[64] file[128]
 *   magic_cookie[4] = 99.130.83.99
 *   options...
 */

#define DHCP_OP_REQUEST 1U
#define DHCP_OP_REPLY 2U
#define DHCP_HTYPE_ETHERNET 1U
#define DHCP_HLEN_ETHERNET 6U
#define DHCP_MSG_MIN_LEN 240U
#define DHCP_MAGIC_COOKIE_0 99U
#define DHCP_MAGIC_COOKIE_1 130U
#define DHCP_MAGIC_COOKIE_2 83U
#define DHCP_MAGIC_COOKIE_3 99U

/* Option codes */
#define DHCP_OPT_SUBNET_MASK 1U
#define DHCP_OPT_ROUTER 3U
#define DHCP_OPT_MSG_TYPE 53U
#define DHCP_OPT_LEASE_TIME 51U
#define DHCP_OPT_SERVER_ID 54U
#define DHCP_OPT_END 255U

/**
 * @brief Build a minimal DHCP message.
 */
static size_t dhcp_build_message(uint8_t* buf, size_t buf_len, uint8_t op, uint32_t xid,
                                 const uint8_t chaddr[6], const uint8_t yiaddr[4],
                                 const uint8_t siaddr[4], uint8_t msg_type,
                                 const uint8_t* subnet_mask, const uint8_t* gateway) {
  if (buf_len < 300U) {
    return 0U;
  }

  memset(buf, 0, 300U);

  buf[0] = op;
  buf[1] = DHCP_HTYPE_ETHERNET;
  buf[2] = DHCP_HLEN_ETHERNET;
  buf[3] = 0U; /* hops */
  WRITE_U32(buf, 4U, xid);
  /* secs=0, flags=0x8000 (broadcast) */
  WRITE_U16(buf, 10U, 0x8000U);

  if (yiaddr != NULL) {
    memcpy(buf + 16U, yiaddr, 4U);
  }
  if (siaddr != NULL) {
    memcpy(buf + 20U, siaddr, 4U);
  }
  if (chaddr != NULL) {
    memcpy(buf + 28U, chaddr, 6U);
  }

  /* Magic cookie */
  buf[236] = DHCP_MAGIC_COOKIE_0;
  buf[237] = DHCP_MAGIC_COOKIE_1;
  buf[238] = DHCP_MAGIC_COOKIE_2;
  buf[239] = DHCP_MAGIC_COOKIE_3;

  /* Options */
  size_t off = 240U;

  /* Option 53: Message Type */
  buf[off++] = DHCP_OPT_MSG_TYPE;
  buf[off++] = 1U;
  buf[off++] = msg_type;

  if (subnet_mask != NULL) {
    buf[off++] = DHCP_OPT_SUBNET_MASK;
    buf[off++] = 4U;
    memcpy(buf + off, subnet_mask, 4U);
    off += 4U;
  }

  if (gateway != NULL) {
    buf[off++] = DHCP_OPT_ROUTER;
    buf[off++] = 4U;
    memcpy(buf + off, gateway, 4U);
    off += 4U;
  }

  if (siaddr != NULL) {
    buf[off++] = DHCP_OPT_SERVER_ID;
    buf[off++] = 4U;
    memcpy(buf + off, siaddr, 4U);
    off += 4U;
  }

  /* Lease time: 86400 seconds (1 day) */
  buf[off++] = DHCP_OPT_LEASE_TIME;
  buf[off++] = 4U;
  WRITE_U32(buf, (unsigned)off, 86400U);
  off += 4U;

  buf[off++] = DHCP_OPT_END;

  return off;
}

/**
 * @brief Extract the DHCP message type from options.
 */
static uint8_t dhcp_get_msg_type(const uint8_t* buf, size_t len) {
  if (len < 241U) {
    return 0U;
  }
  size_t off = 240U;
  while (off < len) {
    uint8_t opt = buf[off++];
    if (opt == DHCP_OPT_END) {
      break;
    }
    if (off >= len) {
      break;
    }
    uint8_t opt_len = buf[off++];
    if (opt == DHCP_OPT_MSG_TYPE && opt_len >= 1U && off < len) {
      return buf[off];
    }
    off += opt_len;
  }
  return 0U;
}

int dhcp_client_discover(Node* node) {
  if (node == NULL) {
    magi_errno = MAGI_ERR_BADARGS;
    return MAGI_ERR_BADARGS;
  }

  /* Get the host's MAC address from interface 1 */
  Interface* iface = node_get_interface(node, 1U);
  if (iface == NULL) {
    LOG(node->name, "DHCP: no interface available");
    return MAGI_ERR_BADARGS;
  }

  /* Create UDP socket and bind to port 68 */
  MagiSocket* sock = magi_socket(node, MAGI_AF_INET, MAGI_SOCK_DGRAM);
  if (sock == NULL) {
    LOG(node->name, "DHCP: failed to create socket");
    return MAGI_ERR_NOMEM;
  }

  int status = magi_bind(sock, "0.0.0.0", DHCP_CLIENT_PORT);
  if (status != MAGI_OK) {
    LOG(node->name, "DHCP: failed to bind to port 68");
    magi_close(sock);
    return status;
  }

  /* Generate transaction ID */
  uint32_t xid = (uint32_t)(rand() & 0xFFFFFFFF);

  /* Build DISCOVER message */
  uint8_t msg_buf[300];
  size_t msg_len = dhcp_build_message(msg_buf, sizeof(msg_buf), DHCP_OP_REQUEST, xid, iface->mac,
                                      NULL, NULL, DHCP_DISCOVER, NULL, NULL);
  if (msg_len == 0U) {
    LOG(node->name, "DHCP: failed to build DISCOVER");
    magi_close(sock);
    return MAGI_ERR_BADARGS;
  }

  LOG(node->name, "DHCP: sending DISCOVER (xid=0x%08X)", (unsigned)xid);
  status = magi_sendto(sock, msg_buf, msg_len, "255.255.255.255", DHCP_SERVER_PORT);
  if (status != MAGI_OK) {
    LOG(node->name, "DHCP: failed to send DISCOVER");
    magi_close(sock);
    return status;
  }

  /* In sequential mode, the OFFER should already be in our buffer */
  if (!magi_has_data(sock)) {
    LOG(node->name, "DHCP: no OFFER received");
    magi_close(sock);
    return MAGI_ERR_TIMEOUT;
  }

  uint8_t recv_buf[512];
  int rd = magi_recv(sock, recv_buf, sizeof(recv_buf));
  if (rd <= 0) {
    LOG(node->name, "DHCP: failed to receive OFFER");
    magi_close(sock);
    return MAGI_ERR_TIMEOUT;
  }

  uint8_t offer_type = dhcp_get_msg_type(recv_buf, (size_t)rd);
  if (offer_type != DHCP_OFFER) {
    LOG(node->name, "DHCP: expected OFFER, got type %u", (unsigned)offer_type);
    magi_close(sock);
    return MAGI_ERR_BADARGS;
  }

  /* Extract yiaddr and siaddr from OFFER */
  uint8_t offered_ip[4];
  uint8_t server_ip[4];
  memcpy(offered_ip, recv_buf + 16U, 4U);
  memcpy(server_ip, recv_buf + 20U, 4U);

  char offered_ip_str[16];
  char server_ip_str[16];
  ipv4_address_to_string(offered_ip, offered_ip_str);
  ipv4_address_to_string(server_ip, server_ip_str);
  LOG(node->name, "DHCP: received OFFER — IP %s from server %s", offered_ip_str, server_ip_str);

  /* Send REQUEST */
  msg_len = dhcp_build_message(msg_buf, sizeof(msg_buf), DHCP_OP_REQUEST, xid, iface->mac,
                               offered_ip, server_ip, DHCP_REQUEST, NULL, NULL);
  LOG(node->name, "DHCP: sending REQUEST for %s", offered_ip_str);
  status = magi_sendto(sock, msg_buf, msg_len, "255.255.255.255", DHCP_SERVER_PORT);
  if (status != MAGI_OK) {
    LOG(node->name, "DHCP: failed to send REQUEST");
    magi_close(sock);
    return status;
  }

  /* Receive ACK */
  if (!magi_has_data(sock)) {
    LOG(node->name, "DHCP: no ACK received");
    magi_close(sock);
    return MAGI_ERR_TIMEOUT;
  }

  rd = magi_recv(sock, recv_buf, sizeof(recv_buf));
  if (rd <= 0) {
    LOG(node->name, "DHCP: failed to receive ACK");
    magi_close(sock);
    return MAGI_ERR_TIMEOUT;
  }

  uint8_t ack_type = dhcp_get_msg_type(recv_buf, (size_t)rd);
  if (ack_type != DHCP_ACK) {
    LOG(node->name, "DHCP: expected ACK, got type %u", (unsigned)ack_type);
    magi_close(sock);
    return MAGI_ERR_BADARGS;
  }

  /* Configure the host's IP address */
  snprintf(iface->ip_address, sizeof(iface->ip_address), "%s", offered_ip_str);
  LOG(node->name, "DHCP: ACK received — host configured with IP %s", offered_ip_str);

  magi_close(sock);
  return MAGI_OK;
}

int dhcp_server_start(Node* node, const char* pool_start, const char* pool_end,
                      const char* subnet_mask, const char* gateway) {
  (void)pool_end; /* simplified: only serve pool_start */

  if (node == NULL || pool_start == NULL || subnet_mask == NULL || gateway == NULL) {
    magi_errno = MAGI_ERR_BADARGS;
    return MAGI_ERR_BADARGS;
  }

  LOG(node->name, "DHCP server: pool %s-%s mask %s gw %s", pool_start,
      pool_end != NULL ? pool_end : pool_start, subnet_mask, gateway);

  /* Create a UDP socket bound to port 67 */
  MagiSocket* sock = magi_socket(node, MAGI_AF_INET, MAGI_SOCK_DGRAM);
  if (sock == NULL) {
    return MAGI_ERR_NOMEM;
  }

  Interface* iface = node_get_interface(node, 1U);
  const char* bind_ip = (iface != NULL && iface->ip_address[0] != '\0') ? iface->ip_address : "0.0.0.0";

  /* Parse bind IP (strip CIDR prefix if present) */
  uint8_t bind_bytes[4];
  if (ipv4_parse_address(bind_ip, bind_bytes) != MAGI_OK) {
    magi_close(sock);
    return MAGI_ERR_BADARGS;
  }
  char bind_str[16];
  ipv4_address_to_string(bind_bytes, bind_str);

  int status = magi_bind(sock, bind_str, DHCP_SERVER_PORT);
  if (status != MAGI_OK) {
    LOG(node->name, "DHCP server: failed to bind port 67");
    magi_close(sock);
    return status;
  }

  LOG(node->name, "DHCP server: listening on port 67");

  /* In sequential mode, wait for DISCOVER in recv buffer */
  if (!magi_has_data(sock)) {
    /* No immediate client — server stays bound for future packets */
    LOG(node->name, "DHCP server: ready (no clients yet)");
    /* Note: sock is intentionally NOT closed. It stays bound. */
    return MAGI_OK;
  }

  /* Handle DISCOVER */
  uint8_t recv_buf[512];
  int rd = magi_recv(sock, recv_buf, sizeof(recv_buf));
  if (rd <= 0) {
    return MAGI_OK;
  }

  uint8_t msg_type = dhcp_get_msg_type(recv_buf, (size_t)rd);
  if (msg_type != DHCP_DISCOVER) {
    LOG(node->name, "DHCP server: expected DISCOVER, got %u", (unsigned)msg_type);
    return MAGI_OK;
  }

  uint32_t xid = READ_U32(recv_buf, 4U);
  uint8_t client_mac[6];
  memcpy(client_mac, recv_buf + 28U, 6U);

  /* Build OFFER */
  uint8_t pool_ip[4];
  uint8_t mask_bytes[4];
  uint8_t gw_bytes[4];
  uint8_t server_bytes[4];
  ipv4_parse_address(pool_start, pool_ip);
  ipv4_parse_address(subnet_mask, mask_bytes);
  ipv4_parse_address(gateway, gw_bytes);
  memcpy(server_bytes, bind_bytes, 4U);

  uint8_t offer_buf[300];
  size_t offer_len = dhcp_build_message(offer_buf, sizeof(offer_buf), DHCP_OP_REPLY, xid,
                                        client_mac, pool_ip, server_bytes, DHCP_OFFER, mask_bytes,
                                        gw_bytes);

  char client_ip_str[16];
  ipv4_address_to_string(pool_ip, client_ip_str);

  /* Send OFFER back to client port */
  LOG(node->name, "DHCP server: sending OFFER %s", client_ip_str);
  (void)magi_sendto(sock, offer_buf, offer_len, "255.255.255.255", DHCP_CLIENT_PORT);

  /* Wait for REQUEST */
  if (!magi_has_data(sock)) {
    return MAGI_OK;
  }

  rd = magi_recv(sock, recv_buf, sizeof(recv_buf));
  if (rd <= 0) {
    return MAGI_OK;
  }

  msg_type = dhcp_get_msg_type(recv_buf, (size_t)rd);
  if (msg_type != DHCP_REQUEST) {
    LOG(node->name, "DHCP server: expected REQUEST, got %u", (unsigned)msg_type);
    return MAGI_OK;
  }

  /* Build ACK */
  uint8_t ack_buf[300];
  size_t ack_len = dhcp_build_message(ack_buf, sizeof(ack_buf), DHCP_OP_REPLY, xid, client_mac,
                                      pool_ip, server_bytes, DHCP_ACK, mask_bytes, gw_bytes);

  LOG(node->name, "DHCP server: sending ACK for %s", client_ip_str);
  (void)magi_sendto(sock, ack_buf, ack_len, "255.255.255.255", DHCP_CLIENT_PORT);

  return MAGI_OK;
}
