#define _POSIX_C_SOURCE 200809L

#include "dns.h"

#include "core/interface.h"
#include "layer3/ipv4.h"
#include "layer7/magi_socket.h"
#include "utils/byteops.h"
#include "utils/log.h"
#include "utils/magi_error.h"

#include <stdlib.h>
#include <string.h>

/**
 * @brief Build a simplified DNS query message.
 *
 * Wire: query_id[2] | qr[1]=0 | qname[N]\0 | qtype[2]
 */
static size_t dns_build_query(uint8_t* buf, size_t buf_len, uint16_t query_id,
                              const char* hostname) {
  size_t name_len = strlen(hostname) + 1U; /* including null */
  size_t total = 2U + 1U + name_len + 2U;  /* id + qr + qname\0 + qtype */

  if (buf_len < total) {
    return 0U;
  }

  WRITE_U16(buf, 0U, query_id);
  buf[2] = 0U; /* qr=0 (query) */
  memcpy(buf + 3U, hostname, name_len);
  WRITE_U16(buf, (unsigned)(3U + name_len), DNS_QTYPE_A);

  return total;
}

/**
 * @brief Build a simplified DNS response message.
 *
 * Wire: query_id[2] | qr[1]=1 | qname[N]\0 | qtype[2] | rdata[4]
 */
static size_t dns_build_response(uint8_t* buf, size_t buf_len, uint16_t query_id,
                                 const char* hostname, const uint8_t rdata[4]) {
  size_t name_len = strlen(hostname) + 1U;
  size_t total = 2U + 1U + name_len + 2U + 4U;

  if (buf_len < total) {
    return 0U;
  }

  WRITE_U16(buf, 0U, query_id);
  buf[2] = 1U; /* qr=1 (response) */
  memcpy(buf + 3U, hostname, name_len);
  WRITE_U16(buf, (unsigned)(3U + name_len), DNS_QTYPE_A);
  memcpy(buf + 3U + name_len + 2U, rdata, 4U);

  return total;
}

int dns_query(Node* node, const char* dns_server_ip, const char* hostname, uint8_t ip_out[4]) {
  if (node == NULL || dns_server_ip == NULL || hostname == NULL || ip_out == NULL) {
    magi_errno = MAGI_ERR_BADARGS;
    return MAGI_ERR_BADARGS;
  }

  MagiSocket* sock = magi_socket(node, MAGI_AF_INET, MAGI_SOCK_DGRAM);
  if (sock == NULL) {
    return MAGI_ERR_NOMEM;
  }

  /* Bind to an ephemeral port */
  Interface* iface = node_get_interface(node, 1U);
  const char* local_ip_str = "0.0.0.0";
  if (iface != NULL && iface->ip_address[0] != '\0') {
    uint8_t lip[4];
    if (ipv4_parse_address(iface->ip_address, lip) == MAGI_OK) {
      static char lip_str[16];
      ipv4_address_to_string(lip, lip_str);
      local_ip_str = lip_str;
    }
  }

  uint16_t ephemeral = (uint16_t)(49152U + ((uintptr_t)sock & 0x3FFFU));
  int status = magi_bind(sock, local_ip_str, ephemeral);
  if (status != MAGI_OK) {
    magi_close(sock);
    return status;
  }

  /* Build query */
  uint16_t query_id = (uint16_t)(rand() & 0xFFFF);
  uint8_t query_buf[256];
  size_t query_len = dns_build_query(query_buf, sizeof(query_buf), query_id, hostname);
  if (query_len == 0U) {
    magi_close(sock);
    return MAGI_ERR_BADARGS;
  }

  LOG(node->name, "DNS: query %s @%s (id=%u)", hostname, dns_server_ip, (unsigned)query_id);
  status = magi_sendto(sock, query_buf, query_len, dns_server_ip, DNS_PORT);
  if (status != MAGI_OK) {
    LOG(node->name, "DNS: failed to send query");
    magi_close(sock);
    return status;
  }

  /* Receive response */
  if (!magi_has_data(sock)) {
    LOG(node->name, "DNS: no response received");
    magi_close(sock);
    return MAGI_ERR_TIMEOUT;
  }

  uint8_t resp_buf[256];
  int rd = magi_recv(sock, resp_buf, sizeof(resp_buf));
  if (rd < 5) {
    LOG(node->name, "DNS: response too short");
    magi_close(sock);
    return MAGI_ERR_BADARGS;
  }

  /* Parse response: check qr=1 and extract rdata */
  uint16_t resp_id = READ_U16(resp_buf, 0U);
  uint8_t qr = resp_buf[2];

  if (resp_id != query_id || qr != 1U) {
    LOG(node->name, "DNS: bad response (id=%u qr=%u)", (unsigned)resp_id, (unsigned)qr);
    magi_close(sock);
    return MAGI_ERR_BADARGS;
  }

  /* Find the rdata at the end: skip qname\0 + qtype[2] */
  size_t name_start = 3U;
  size_t name_end = name_start;
  while (name_end < (size_t)rd && resp_buf[name_end] != '\0') {
    name_end++;
  }
  name_end++; /* skip the null */

  size_t rdata_off = name_end + 2U; /* skip qtype */
  if (rdata_off + 4U > (size_t)rd) {
    LOG(node->name, "DNS: response missing rdata");
    magi_close(sock);
    return MAGI_ERR_BADARGS;
  }

  memcpy(ip_out, resp_buf + rdata_off, 4U);

  char resolved[16];
  ipv4_address_to_string(ip_out, resolved);
  LOG(node->name, "DNS: %s resolved to %s", hostname, resolved);

  magi_close(sock);
  return MAGI_OK;
}

int dns_server_start(Node* node, HashMap* records) {
  if (node == NULL || records == NULL) {
    magi_errno = MAGI_ERR_BADARGS;
    return MAGI_ERR_BADARGS;
  }

  MagiSocket* sock = magi_socket(node, MAGI_AF_INET, MAGI_SOCK_DGRAM);
  if (sock == NULL) {
    return MAGI_ERR_NOMEM;
  }

  Interface* iface = node_get_interface(node, 1U);
  const char* bind_ip = "0.0.0.0";
  if (iface != NULL && iface->ip_address[0] != '\0') {
    uint8_t lip[4];
    if (ipv4_parse_address(iface->ip_address, lip) == MAGI_OK) {
      static char lip_str[16];
      ipv4_address_to_string(lip, lip_str);
      bind_ip = lip_str;
    }
  }

  int status = magi_bind(sock, bind_ip, DNS_PORT);
  if (status != MAGI_OK) {
    LOG(node->name, "DNS server: failed to bind port 53");
    magi_close(sock);
    return status;
  }

  LOG(node->name, "DNS server: listening on port 53");

  /* In sequential mode, handle any immediately available query */
  if (!magi_has_data(sock)) {
    LOG(node->name, "DNS server: ready (no queries yet)");
    return MAGI_OK;
  }

  uint8_t recv_buf[256];
  char sender_ip[16];
  uint16_t sender_port = 0U;
  int rd = magi_recvfrom(sock, recv_buf, sizeof(recv_buf), sender_ip, &sender_port);
  if (rd < 5) {
    return MAGI_OK;
  }

  /* Parse query */
  uint16_t query_id = READ_U16(recv_buf, 0U);
  uint8_t qr = recv_buf[2];
  if (qr != 0U) {
    return MAGI_OK; /* not a query */
  }

  /* Extract qname */
  const char* qname = (const char*)(recv_buf + 3U);
  LOG(node->name, "DNS server: query for '%s' (id=%u)", qname, (unsigned)query_id);

  /* Look up in records */
  const char* ip_str = hashmap_get(records, qname);
  if (ip_str == NULL) {
    LOG(node->name, "DNS server: no record for '%s'", qname);
    return MAGI_OK;
  }

  uint8_t rdata[4];
  if (ipv4_parse_address(ip_str, rdata) != MAGI_OK) {
    LOG(node->name, "DNS server: bad IP in record for '%s'", qname);
    return MAGI_OK;
  }

  /* Build and send response */
  uint8_t resp_buf[256];
  size_t resp_len = dns_build_response(resp_buf, sizeof(resp_buf), query_id, qname, rdata);

  LOG(node->name, "DNS server: responding %s → %s", qname, ip_str);
  (void)magi_sendto(sock, resp_buf, resp_len, sender_ip, sender_port);

  return MAGI_OK;
}
