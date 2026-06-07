/**
 * @file ipv4.h
 * @brief IPv4 packet serialization and host-side ICMP helpers.
 */

#ifndef MAGI_LAYER3_IPV4_H
#define MAGI_LAYER3_IPV4_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "core/node.h"
#include "core/packet.h"

#define IPV4_HEADER_LEN 20U
#define IPV4_VERSION_IHL 0x45U
#define IPV4_DEFAULT_TTL 64U
#define IPV4_PROTOCOL_ICMP 1U
#define IPV4_PROTOCOL_TCP 6U
#define IPV4_PROTOCOL_UDP 17U
#define IPV4_ETHERTYPE 0x0800U

typedef struct IPv4Packet {
  Packet base;
  uint8_t version_ihl;
  uint8_t tos;
  uint16_t total_len;
  uint16_t identification;
  uint16_t flags_frag_off;
  uint8_t ttl;
  uint8_t protocol;
  uint16_t checksum;
  uint8_t src_ip[4];
  uint8_t dst_ip[4];
  const uint8_t* payload;
  size_t payload_len;
} IPv4Packet;

int ipv4_pack(IPv4Packet* pkt, uint8_t* out, size_t out_len);
int ipv4_unpack(IPv4Packet* pkt, const uint8_t* in, size_t in_len);
int ipv4_packet_to_bytes(IPv4Packet* pkt, uint8_t** bytes_out, size_t* len_out);

int ipv4_parse_address(const char* text, uint8_t out[4]);
int ipv4_parse_cidr(const char* text, uint8_t ip_out[4], uint8_t network_out[4],
                    uint8_t mask_out[4], int* prefix_len_out);
void ipv4_address_to_string(const uint8_t ip[4], char out[16]);
int ipv4_format_cidr(const uint8_t network[4], int prefix_len, char* out, size_t out_len);
bool ipv4_addr_equal(const uint8_t lhs[4], const uint8_t rhs[4]);
bool ipv4_addr_is_zero(const uint8_t ip[4]);
bool ipv4_addr_in_network(const uint8_t ip[4], const uint8_t network[4], const uint8_t mask[4]);

int ipv4_host_attach(Node* node);
int ipv4_send_packet(Node* node, const uint8_t src_ip[4], const uint8_t dst_ip[4], uint8_t protocol,
                     uint8_t ttl, const uint8_t* data, size_t len);
int ipv4_host_ping(Node* node, const char* target_ip);
int ipv4_host_traceroute(Node* node, const char* target_ip, uint8_t max_hops);

#endif
