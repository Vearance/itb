#define _POSIX_C_SOURCE 200809L

#include "nat.h"

#include "layer3/ipv4.h"
#include "utils/byteops.h"
#include "utils/hashmap.h"
#include "utils/log.h"
#include "utils/magi_error.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define IPV4_PSEUDO_HDR_LEN 12U
#define ICMP_HEADER_MIN_LEN 8U
#define ICMP_CKSUM_OFF 2U
#define ICMP_ID_OFF 4U
#define NAT_INITIAL_CAPACITY 16U

/* ─── Key building helpers ─── */

/**
 * @brief Build a forward-map key: "proto:pub_port"
 */
static void nat_fwd_key(uint8_t protocol, uint16_t pub_port, char out[24]) {
  snprintf(out, 24U, "%u:%u", (unsigned)protocol, (unsigned)pub_port);
}

/**
 * @brief Build a reverse-map key: "proto:priv_ip:priv_port"
 */
static void nat_rev_key(uint8_t protocol, const uint8_t priv_ip[4], uint16_t priv_port,
                        char out[32]) {
  char ip_str[16];
  ipv4_address_to_string(priv_ip, ip_str);
  snprintf(out, 32U, "%u:%s:%u", (unsigned)protocol, ip_str, (unsigned)priv_port);
}

/**
 * @brief Free a NATEntry (hashmap_foreach callback).
 */
static void free_nat_entry(const char* key, void* value, void* ctx) {
  (void)key;
  (void)ctx;
  free(value);
}

/* ─── Public API ─── */

NATTable* nat_table_new(const uint8_t public_ip[4]) {
  if (public_ip == NULL) {
    magi_errno = MAGI_ERR_BADARGS;
    return NULL;
  }

  NATTable* t = calloc(1U, sizeof(*t));
  if (t == NULL) {
    magi_errno = MAGI_ERR_NOMEM;
    return NULL;
  }

  memcpy(t->public_ip, public_ip, 4U);
  t->forward = hashmap_new(NAT_INITIAL_CAPACITY);
  t->reverse = hashmap_new(NAT_INITIAL_CAPACITY);
  t->next_port = NAT_MIN_PORT;

  if (t->forward == NULL || t->reverse == NULL) {
    hashmap_free(t->forward);
    hashmap_free(t->reverse);
    free(t);
    magi_errno = MAGI_ERR_NOMEM;
    return NULL;
  }

  return t;
}

void nat_table_free(NATTable* t) {
  if (t == NULL) {
    return;
  }

  hashmap_foreach(t->forward, free_nat_entry, NULL);
  hashmap_free(t->forward);
  hashmap_free(t->reverse);
  free(t);
}

/**
 * @brief Allocate a new public port and create a session entry.
 *
 * @param t         NAT table.
 * @param protocol  IP protocol number.
 * @param priv_ip   Private IP address.
 * @param priv_port Private port (0 for ICMP).
 * @param entry_out Output for the allocated entry.
 * @return MAGI_OK on success, MAGI_ERR_PORTUSED if ports exhausted.
 */
static int nat_allocate_port(NATTable* t, uint8_t protocol, const uint8_t priv_ip[4],
                             uint16_t priv_port, NATEntry** entry_out) {
  uint16_t start = t->next_port;
  uint16_t port = start;

  do {
    char fwd_key[24];
    nat_fwd_key(protocol, port, fwd_key);

    if (hashmap_get(t->forward, fwd_key) == NULL) {
      /* Port available — allocate */
      NATEntry* entry = calloc(1U, sizeof(*entry));
      if (entry == NULL) {
        magi_errno = MAGI_ERR_NOMEM;
        return MAGI_ERR_NOMEM;
      }

      memcpy(entry->private_ip, priv_ip, 4U);
      entry->private_port = priv_port;
      entry->public_port = port;
      entry->protocol = protocol;

      char rev_key[32];
      nat_rev_key(protocol, priv_ip, priv_port, rev_key);
      nat_fwd_key(protocol, port, fwd_key);

      if (hashmap_set(t->forward, fwd_key, entry) != MAGI_OK ||
          hashmap_set(t->reverse, rev_key, entry) != MAGI_OK) {
        free(entry);
        magi_errno = MAGI_ERR_NOMEM;
        return MAGI_ERR_NOMEM;
      }

      t->next_port = port >= NAT_MAX_PORT ? NAT_MIN_PORT : (uint16_t)(port + 1U);
      *entry_out = entry;
      return MAGI_OK;
    }

    port = port >= NAT_MAX_PORT ? NAT_MIN_PORT : (uint16_t)(port + 1U);
  } while (port != start);

  magi_errno = MAGI_ERR_PORTUSED;
  return MAGI_ERR_PORTUSED;
}

/**
 * @brief Recompute the IPv4 header checksum on the parsed struct fields only.
 *
 * NOTE: This function modifies pkt->checksum but does NOT modify any
 * serialized buffer. The checksum will be computed correctly when
 * ipv4_packet_to_bytes() / ipv4_pack() is called later to serialize
 * the packet for transmission. This avoids crashing on pkt->base.buf
 * which may be NULL when the packet was unpacked from raw bytes.
 */
static void nat_mark_ipv4_cksum_stale(IPv4Packet* pkt) {
  if (pkt == NULL) {
    return;
  }
  /* Zero the checksum field so ipv4_pack() will recompute it. */
  pkt->checksum = 0U;
}

/**
 * @brief Recompute the ICMP checksum in-place after modifying the identifier.
 *
 * ICMP checksum covers only the ICMP message (header + data), with no
 * pseudo-header (unlike TCP/UDP). The checksum field is at byte offset 2.
 */
static void nat_recompute_icmp_cksum(uint8_t* icmp_msg, size_t icmp_len) {
  if (icmp_msg == NULL || icmp_len < ICMP_HEADER_MIN_LEN) {
    return;
  }
  WRITE_U16(icmp_msg, ICMP_CKSUM_OFF, 0U);
  uint16_t cksum = ipv4_checksum(icmp_msg, icmp_len);
  WRITE_U16(icmp_msg, ICMP_CKSUM_OFF, cksum);
}

/**
 * @brief Translate an outbound TCP or UDP packet (private → public).
 *
 * Reads the source port from the transport header, creates or reuses a
 * NAT session, rewrites src_ip → public_ip and src_port → allocated public
 * port. Marks the IPv4 checksum as stale (will be recomputed during
 * serialization).
 */
static int nat_translate_out_tcp_udp(NATTable* t, IPv4Packet* pkt) {
  uint16_t priv_port = READ_U16(pkt->payload, 0U);
  uint16_t dst_port = READ_U16(pkt->payload, 2U);

  /* Check if a reverse entry already exists (reuse session) */
  char rev_key[32];
  nat_rev_key(pkt->protocol, pkt->src_ip, priv_port, rev_key);
  NATEntry* existing = hashmap_get(t->reverse, rev_key);

  if (existing != NULL) {
    /* Reuse existing session — just rewrite IP and port */
    memcpy(pkt->src_ip, t->public_ip, 4U);
    WRITE_U16((uint8_t*)pkt->payload, 0U, existing->public_port);
    nat_mark_ipv4_cksum_stale(pkt);
    return MAGI_OK;
  }

  /* Allocate new public port */
  NATEntry* entry = NULL;
  int status = nat_allocate_port(t, pkt->protocol, pkt->src_ip, priv_port, &entry);
  if (status != MAGI_OK) {
    char src_text[16], dst_text[16];
    ipv4_address_to_string(pkt->src_ip, src_text);
    ipv4_address_to_string(pkt->dst_ip, dst_text);
    LOG("NAT", "Outbound %s:%u -> %s:%u: no ports available", src_text, (unsigned)priv_port,
        dst_text, (unsigned)dst_port);
    return status;
  }

  /* Rewrite source IP and port */
  memcpy(pkt->src_ip, t->public_ip, 4U);
  WRITE_U16((uint8_t*)pkt->payload, 0U, entry->public_port);
  nat_mark_ipv4_cksum_stale(pkt);

  {
    char src_text[16], dst_text[16];
    ipv4_address_to_string(pkt->src_ip, src_text);
    ipv4_address_to_string(pkt->dst_ip, dst_text);
    LOG("NAT", "Outbound %s:%u -> %s:%u mapped to port %u", src_text, (unsigned)priv_port,
        dst_text, (unsigned)dst_port, (unsigned)entry->public_port);
  }

  return MAGI_OK;
}

/**
 * @brief Translate an outbound ICMP Echo Request (private → public).
 *
 * Uses the ICMP identifier (bytes 4-5 of the ICMP header) as a port
 * surrogate. Rewrites src_ip → public_ip and the ICMP identifier →
 * allocated public port so that inbound Echo Replies can be demultiplexed.
 * Recomputes the ICMP checksum since the identifier changed.
 */
static int nat_translate_out_icmp(NATTable* t, IPv4Packet* pkt) {
  if (pkt->payload_len < ICMP_HEADER_MIN_LEN) {
    magi_errno = MAGI_ERR_BADARGS;
    return MAGI_ERR_BADARGS;
  }

  uint16_t icmp_id = READ_U16(pkt->payload, ICMP_ID_OFF);

  /* Check if a reverse entry already exists (reuse session) */
  char rev_key[32];
  nat_rev_key(pkt->protocol, pkt->src_ip, icmp_id, rev_key);
  NATEntry* existing = hashmap_get(t->reverse, rev_key);

  if (existing != NULL) {
    /* Reuse existing session — rewrite src_ip and ICMP identifier */
    memcpy(pkt->src_ip, t->public_ip, 4U);
    WRITE_U16((uint8_t*)pkt->payload, ICMP_ID_OFF, existing->public_port);
    nat_recompute_icmp_cksum((uint8_t*)pkt->payload, pkt->payload_len);
    nat_mark_ipv4_cksum_stale(pkt);
    return MAGI_OK;
  }

  /* Allocate new public port */
  NATEntry* entry = NULL;
  int status = nat_allocate_port(t, pkt->protocol, pkt->src_ip, icmp_id, &entry);
  if (status != MAGI_OK) {
    char src_text[16];
    ipv4_address_to_string(pkt->src_ip, src_text);
    LOG("NAT", "Outbound ICMP %s id=%u: no ports available", src_text, (unsigned)icmp_id);
    return status;
  }

  /* Rewrite source IP and ICMP identifier */
  memcpy(pkt->src_ip, t->public_ip, 4U);
  WRITE_U16((uint8_t*)pkt->payload, ICMP_ID_OFF, entry->public_port);
  nat_recompute_icmp_cksum((uint8_t*)pkt->payload, pkt->payload_len);
  nat_mark_ipv4_cksum_stale(pkt);

  {
    char src_text[16];
    ipv4_address_to_string(pkt->src_ip, src_text);
    LOG("NAT", "Outbound ICMP %s id=%u mapped to port %u", src_text, (unsigned)icmp_id,
        (unsigned)entry->public_port);
  }

  return MAGI_OK;
}

int nat_translate_out(NATTable* t, struct IPv4Packet* pkt) {
  if (t == NULL || pkt == NULL || pkt->payload == NULL || pkt->payload_len < 4U) {
    magi_errno = MAGI_ERR_BADARGS;
    return MAGI_ERR_BADARGS;
  }

  if (pkt->protocol == NAT_PROTO_ICMP) {
    return nat_translate_out_icmp(t, pkt);
  }

  if (pkt->protocol != NAT_PROTO_TCP && pkt->protocol != NAT_PROTO_UDP) {
    magi_errno = MAGI_ERR_BADARGS;
    return MAGI_ERR_BADARGS;
  }

  return nat_translate_out_tcp_udp(t, pkt);
}

/**
 * @brief Translate an inbound TCP or UDP packet (public → private).
 *
 * Looks up the destination port in the forward map, rewrites dst_ip and
 * dst_port back to the original private values. Removes the session entry
 * from both maps (one-shot). Marks the IPv4 checksum as stale (will be
 * recomputed during serialization).
 */
static int nat_translate_in_tcp_udp(NATTable* t, IPv4Packet* pkt) {
  uint16_t dst_port = READ_U16(pkt->payload, 2U);

  char fwd_key[24];
  nat_fwd_key(pkt->protocol, dst_port, fwd_key);
  NATEntry* entry = hashmap_get(t->forward, fwd_key);

  if (entry == NULL) {
    char dst_text[16];
    ipv4_address_to_string(pkt->dst_ip, dst_text);
    LOG("NAT", "Inbound %s:%u: no matching session", dst_text, (unsigned)dst_port);
    magi_errno = MAGI_ERR_BADARGS;
    return MAGI_ERR_BADARGS;
  }

  /* Rewrite destination IP and port back to private values */
  memcpy(pkt->dst_ip, entry->private_ip, 4U);
  WRITE_U16((uint8_t*)pkt->payload, 2U, entry->private_port);
  nat_mark_ipv4_cksum_stale(pkt);

  {
    char orig_dst_text[16], priv_text[16];
    ipv4_address_to_string(t->public_ip, orig_dst_text);
    ipv4_address_to_string(entry->private_ip, priv_text);
    LOG("NAT", "Inbound %s:%u -> %s:%u (was %s:%u via port %u)", orig_dst_text,
        (unsigned)dst_port, priv_text, (unsigned)entry->private_port, orig_dst_text,
        (unsigned)dst_port, (unsigned)entry->public_port);
  }

  /* Remove session from both maps (one-shot) */
  hashmap_delete(t->forward, fwd_key);
  {
    char rev_key[32];
    nat_rev_key(pkt->protocol, entry->private_ip, entry->private_port, rev_key);
    hashmap_delete(t->reverse, rev_key);
  }
  free(entry);

  return MAGI_OK;
}

/**
 * @brief Translate an inbound ICMP Echo Reply (public → private).
 *
 * Looks up the ICMP identifier (which was rewritten to the allocated public
 * port on outbound) in the forward map. Rewrites dst_ip back to the private
 * IP and restores the original ICMP identifier. Recomputes the ICMP checksum
 * since the identifier changed. Removes the session (one-shot).
 */
static int nat_translate_in_icmp(NATTable* t, IPv4Packet* pkt) {
  if (pkt->payload_len < ICMP_HEADER_MIN_LEN) {
    magi_errno = MAGI_ERR_BADARGS;
    return MAGI_ERR_BADARGS;
  }

  uint16_t icmp_id = READ_U16(pkt->payload, ICMP_ID_OFF);

  char fwd_key[24];
  nat_fwd_key(pkt->protocol, icmp_id, fwd_key);
  NATEntry* entry = hashmap_get(t->forward, fwd_key);

  if (entry == NULL) {
    char dst_text[16];
    ipv4_address_to_string(pkt->dst_ip, dst_text);
    LOG("NAT", "Inbound ICMP %s id=%u: no matching session", dst_text, (unsigned)icmp_id);
    magi_errno = MAGI_ERR_BADARGS;
    return MAGI_ERR_BADARGS;
  }

  /* Rewrite destination IP and restore original ICMP identifier */
  memcpy(pkt->dst_ip, entry->private_ip, 4U);
  WRITE_U16((uint8_t*)pkt->payload, ICMP_ID_OFF, entry->private_port);
  nat_recompute_icmp_cksum((uint8_t*)pkt->payload, pkt->payload_len);
  nat_mark_ipv4_cksum_stale(pkt);

  {
    char orig_dst_text[16], priv_text[16];
    ipv4_address_to_string(t->public_ip, orig_dst_text);
    ipv4_address_to_string(entry->private_ip, priv_text);
    LOG("NAT", "Inbound ICMP %s id=%u -> %s id=%u (was %s id=%u via port %u)",
        orig_dst_text, (unsigned)icmp_id, priv_text, (unsigned)entry->private_port,
        orig_dst_text, (unsigned)icmp_id, (unsigned)entry->public_port);
  }

  /* Remove session from both maps (one-shot) */
  hashmap_delete(t->forward, fwd_key);
  {
    char rev_key[32];
    nat_rev_key(pkt->protocol, entry->private_ip, entry->private_port, rev_key);
    hashmap_delete(t->reverse, rev_key);
  }
  free(entry);

  return MAGI_OK;
}

int nat_translate_in(NATTable* t, struct IPv4Packet* pkt) {
  if (t == NULL || pkt == NULL || pkt->payload == NULL || pkt->payload_len < 4U) {
    magi_errno = MAGI_ERR_BADARGS;
    return MAGI_ERR_BADARGS;
  }

  if (pkt->protocol == NAT_PROTO_ICMP) {
    return nat_translate_in_icmp(t, pkt);
  }

  if (pkt->protocol != NAT_PROTO_TCP && pkt->protocol != NAT_PROTO_UDP) {
    magi_errno = MAGI_ERR_BADARGS;
    return MAGI_ERR_BADARGS;
  }

  return nat_translate_in_tcp_udp(t, pkt);
}

/**
 * @brief Context for the nat_print_entry callback.
 */
typedef struct NatPrintCtx {
  const char* name;
  size_t count;
} NatPrintCtx;

/**
 * @brief hashmap_foreach callback to print one NAT session entry.
 */
static void nat_print_entry(const char* key, void* value, void* ctx) {
  NatPrintCtx* pc = (NatPrintCtx*)ctx;
  const NATEntry* entry = (const NATEntry*)value;
  if (entry == NULL || pc == NULL) {
    return;
  }

  (void)key;
  pc->count++;

  const char* proto_str = "?";
  switch (entry->protocol) {
    case NAT_PROTO_ICMP:
      proto_str = "ICMP";
      break;
    case NAT_PROTO_TCP:
      proto_str = "TCP";
      break;
    case NAT_PROTO_UDP:
      proto_str = "UDP";
      break;
    default:
      break;
  }

  char priv_str[16];
  ipv4_address_to_string(entry->private_ip, priv_str);
  LOG(pc->name, "NAT #%zu: %s %s:%u <-> ?:%u", pc->count, proto_str,
      priv_str, (unsigned)entry->private_port, (unsigned)entry->public_port);
}

void nat_print(const NATTable* t, const char* name) {
  if (t == NULL || name == NULL) {
    return;
  }

  char pub_str[16];
  ipv4_address_to_string(t->public_ip, pub_str);

  LOG(name, "NAT: public IP %s, next port %u, active sessions:", pub_str,
      (unsigned)t->next_port);

  NatPrintCtx ctx;
  ctx.name = name;
  ctx.count = 0U;

  hashmap_foreach(t->forward, nat_print_entry, &ctx);

  if (ctx.count == 0U) {
    LOG(name, "NAT: no active sessions");
  }
}
