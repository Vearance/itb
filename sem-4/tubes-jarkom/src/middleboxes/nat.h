/**
 * @file nat.h
 * @brief Network Address Translation / Port Address Translation.
 *
 * Implements one-to-many NAT (masquerading) with dynamic port allocation
 * for TCP, UDP, and ICMP query messages.
 *
 * Protocol handling:
 * - TCP/UDP: src_ip:src_port → public_ip:allocated_port (port-based PAT)
 * - ICMP:    src_ip:icmp_id  → public_ip:allocated_port (ID-based translation per RFC 3022)
 *
 * Forward map: key = "proto:pub_port"  → NATEntry
 * Reverse map: key = "proto:pvt_ip:pvt_port" → NATEntry
 *
 * Sessions are one-shot (removed on inbound translation) for simplicity.
 * Public ports are allocated starting from 1024, incrementing per session.
 *
 * Checksums are marked stale during translation and recomputed by
 * ipv4_pack() during the subsequent serialization step (the NAT
 * translation runs on parsed struct fields, not serialized buffers).
 * ICMP checksums are recomputed in-place since the identifier changes.
 */

#ifndef MAGI_MIDDLEBOXES_NAT_H
#define MAGI_MIDDLEBOXES_NAT_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

struct IPv4Packet;

#define NAT_PROTO_ICMP 1U
#define NAT_PROTO_TCP  6U
#define NAT_PROTO_UDP  17U

#define NAT_MIN_PORT 1024U
#define NAT_MAX_PORT 65535U

/**
 * @brief One NAT session entry.
 */
typedef struct NATEntry {
  uint8_t  private_ip[4];   /**< Original private IP. */
  uint16_t private_port;    /**< Original private port (0 for ICMP). */
  uint16_t public_port;     /**< Allocated public port. */
  uint8_t  protocol;        /**< IP protocol (TCP, UDP, or ICMP). */
} NATEntry;

/**
 * @brief NAT table with forward/reverse lookup maps.
 */
typedef struct NATTable {
  uint8_t   public_ip[4];   /**< Public IP address to masquerade behind. */
  void     *forward;         /**< HashMap: "proto:pub_port" → NATEntry* */
  void     *reverse;         /**< HashMap: "proto:pvt_ip:pvt_port" → NATEntry* */
  uint16_t  next_port;       /**< Next public port to allocate (starts at 1024). */
} NATTable;

/**
 * @brief Create a new NAT table with a given public IP.
 *
 * @param public_ip 4-byte public IP address.
 * @return New NATTable, or NULL on allocation failure.
 */
NATTable* nat_table_new(const uint8_t public_ip[4]);

/**
 * @brief Free a NAT table and all session entries.
 *
 * @param t NAT table to free. NULL is allowed.
 */
void nat_table_free(NATTable* t);

/**
 * @brief Translate an outbound packet (private → public).
 *
 * TCP/UDP: rewrites src_ip → public_ip and src_port → allocated port.
 * ICMP:    rewrites src_ip → public_ip and ICMP identifier → allocated port.
 *
 * IPv4 checksum is marked stale for subsequent serialization.
 * ICMP checksum is recomputed in-place (identifier changed).
 *
 * @param t   NAT table.
 * @param pkt Parsed IPv4 packet to translate.
 * @return MAGI_OK on success, MAGI_ERR_NOMEM on allocation failure,
 *         MAGI_ERR_BADARGS on invalid input, MAGI_ERR_PORTUSED if
 *         no ports available.
 */
int nat_translate_out(NATTable* t, struct IPv4Packet* pkt);

/**
 * @brief Translate an inbound packet (public → private).
 *
 * TCP/UDP: looks up by dst_port, rewrites dst_ip and dst_port to private values.
 * ICMP:    looks up by ICMP identifier, rewrites dst_ip and identifier.
 *
 * The session is removed from both maps after translation (one-shot).
 * ICMP checksum is recomputed in-place (identifier changed).
 *
 * @param t   NAT table.
 * @param pkt Parsed IPv4 packet to translate.
 * @return MAGI_OK on success, MAGI_ERR_BADARGS if no matching session
 *         or invalid input.
 */
int nat_translate_in(NATTable* t, struct IPv4Packet* pkt);

/**
 * @brief Print all active NAT sessions via LOG().
 *
 * @param t    NAT table to display.
 * @param name Router name for log prefix.
 */
void nat_print(const NATTable* t, const char* name);

#endif /* MAGI_MIDDLEBOXES_NAT_H */
