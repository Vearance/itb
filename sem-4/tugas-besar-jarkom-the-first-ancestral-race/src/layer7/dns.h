/**
 * @file dns.h
 * @brief Simplified DNS client/server over UDP port 53.
 *
 * Wire format (simplified, not RFC 1035):
 *   query_id[2] | qr[1] | qname[N]\0 | qtype[2]
 * Response appends rdata[4] (IPv4 address).
 */

#ifndef MAGI_LAYER7_DNS_H
#define MAGI_LAYER7_DNS_H

#include "core/node.h"
#include "utils/hashmap.h"

#include <stddef.h>
#include <stdint.h>

#define DNS_PORT 53U
#define DNS_QTYPE_A 1U

/**
 * @brief Send a DNS query and receive the resolved IPv4 address.
 *
 * @param node           Host node to send from.
 * @param dns_server_ip  DNS server IPv4 address (dotted decimal).
 * @param hostname       Hostname to resolve.
 * @param ip_out         Output buffer for resolved IPv4 (4 bytes).
 * @return MAGI_OK on success, otherwise an error code.
 */
int dns_query(Node* node, const char* dns_server_ip, const char* hostname, uint8_t ip_out[4]);

/**
 * @brief Start a DNS server on a host node.
 *
 * @param node    Host node to run the server on.
 * @param records HashMap of name → ip_string records.
 * @return MAGI_OK on success, otherwise an error code.
 */
int dns_server_start(Node* node, HashMap* records);

#endif /* MAGI_LAYER7_DNS_H */
