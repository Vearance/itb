#define _POSIX_C_SOURCE 200809L

#include "acl.h"

#include "layer3/ipv4.h"
#include "utils/log.h"
#include "utils/magi_error.h"

#include <stdlib.h>
#include <string.h>
#include <strings.h>

#define ACL_INITIAL_CAPACITY 8U

ACLTable* acl_table_new(void) {
  ACLTable* t = calloc(1U, sizeof(*t));
  if (t == NULL) {
    magi_errno = MAGI_ERR_NOMEM;
    return NULL;
  }

  t->rules = calloc(ACL_INITIAL_CAPACITY, sizeof(ACLRule));
  if (t->rules == NULL) {
    free(t);
    magi_errno = MAGI_ERR_NOMEM;
    return NULL;
  }

  t->count = 0U;
  t->capacity = ACL_INITIAL_CAPACITY;
  return t;
}

void acl_table_free(ACLTable* t) {
  if (t == NULL) {
    return;
  }

  free(t->rules);
  free(t);
}

int acl_add_rule(ACLTable* t, ACLRule rule) {
  if (t == NULL) {
    magi_errno = MAGI_ERR_BADARGS;
    return MAGI_ERR_BADARGS;
  }

  /* Grow the array if full */
  if (t->count >= t->capacity) {
    size_t new_cap = t->capacity * 2U;
    ACLRule* new_rules = realloc(t->rules, new_cap * sizeof(ACLRule));
    if (new_rules == NULL) {
      magi_errno = MAGI_ERR_NOMEM;
      return MAGI_ERR_NOMEM;
    }
    t->rules = new_rules;
    t->capacity = new_cap;
  }

  rule.rule_id = (uint32_t)t->count;
  t->rules[t->count] = rule;
  t->count++;
  return MAGI_OK;
}

void acl_clear(ACLTable* t) {
  if (t == NULL) {
    return;
  }
  t->count = 0U;
}

/**
 * @brief Test whether an IP address matches a CIDR or "any" string.
 *
 * @param ip    4-byte IP address.
 * @param cidr  CIDR string ("x.x.x.x/y" or "any").
 * @return true if the IP matches the CIDR range, false otherwise.
 */
static bool acl_ip_matches_cidr(const uint8_t ip[4], const char* cidr) {
  if (ip == NULL || cidr == NULL) {
    return false;
  }

  if (strcmp(cidr, ACL_CIDR_ANY) == 0) {
    return true;
  }

  uint8_t network[4];
  uint8_t mask[4];
  int prefix_len = 0;
  uint8_t parsed_ip[4];

  if (ipv4_parse_cidr(cidr, parsed_ip, network, mask, &prefix_len) != MAGI_OK) {
    return false;
  }

  return ipv4_addr_in_network(ip, network, mask);
}

/**
 * @brief Parse a protocol name or number from a string.
 *
 * Accepts: "any", "icmp", "tcp", "udp", or a numeric protocol number.
 *
 * @param text Input string.
 * @return Protocol number, or 0xFF on failure.
 */
uint8_t acl_parse_protocol(const char* text) {
  if (text == NULL) {
    return 0xFF;
  }

  if (strcmp(text, "any") == 0 || strcmp(text, "0") == 0) {
    return ACL_PROTO_ANY;
  }
  if (strcasecmp(text, "icmp") == 0) {
    return IPV4_PROTOCOL_ICMP;
  }
  if (strcasecmp(text, "tcp") == 0) {
    return IPV4_PROTOCOL_TCP;
  }
  if (strcasecmp(text, "udp") == 0) {
    return IPV4_PROTOCOL_UDP;
  }

  char* end = NULL;
  long val = strtol(text, &end, 10);
  if (end != text && *end == '\0' && val >= 0 && val <= 255) {
    return (uint8_t)val;
  }

  return 0xFF; /* invalid */
}

int acl_check(const ACLTable* t, const struct IPv4Packet* pkt) {
  if (t == NULL || pkt == NULL) {
    return MAGI_OK; /* No ACL → permit */
  }

  for (size_t i = 0U; i < t->count; i++) {
    const ACLRule* rule = &t->rules[i];

    /* Check source CIDR */
    if (!acl_ip_matches_cidr(pkt->src_ip, rule->src_cidr)) {
      continue;
    }

    /* Check destination CIDR */
    if (!acl_ip_matches_cidr(pkt->dst_ip, rule->dst_cidr)) {
      continue;
    }

    /* Check protocol */
    if (rule->protocol != ACL_PROTO_ANY && rule->protocol != pkt->protocol) {
      continue;
    }

    /* Check source port (only meaningful for TCP/UDP) */
    if (rule->src_port != ACL_PORT_ANY &&
        (pkt->protocol == IPV4_PROTOCOL_TCP || pkt->protocol == IPV4_PROTOCOL_UDP)) {
      /* Extract source port from transport header (first 2 bytes) */
      uint16_t port = 0;
      if (pkt->payload_len >= 2U && pkt->payload != NULL) {
        port = (uint16_t)((uint16_t)pkt->payload[0] << 8) | pkt->payload[1];
      }
      if (port != rule->src_port) {
        continue;
      }
    }

    /* Check destination port (only meaningful for TCP/UDP) */
    if (rule->dst_port != ACL_PORT_ANY &&
        (pkt->protocol == IPV4_PROTOCOL_TCP || pkt->protocol == IPV4_PROTOCOL_UDP)) {
      uint16_t port = 0;
      if (pkt->payload_len >= 4U && pkt->payload != NULL) {
        port = (uint16_t)((uint16_t)pkt->payload[2] << 8) | pkt->payload[3];
      }
      if (port != rule->dst_port) {
        continue;
      }
    }

    /* Rule matched — return its action */
    return rule->permit ? MAGI_OK : MAGI_ERR_ACL_DENY;
  }

  /* No rule matched — default permit */
  return MAGI_OK;
}

void acl_print(const ACLTable* t, const char* name) {
  if (t == NULL || name == NULL) {
    return;
  }

  const char* prefix = name != NULL ? name : "ACL";

  if (t->count == 0U) {
    LOG(prefix, "ACL: no rules (default: permit all)");
    return;
  }

  LOG(prefix, "ACL: %zu rule(s) (default: permit all if no match)", t->count);
  for (size_t i = 0U; i < t->count; i++) {
    const ACLRule* rule = &t->rules[i];

    const char* action = rule->permit ? "PERMIT" : "DENY";
    const char* proto_str = "any";
    char proto_buf[8];

    switch (rule->protocol) {
      case IPV4_PROTOCOL_ICMP:
        proto_str = "icmp";
        break;
      case IPV4_PROTOCOL_TCP:
        proto_str = "tcp";
        break;
      case IPV4_PROTOCOL_UDP:
        proto_str = "udp";
        break;
      case ACL_PROTO_ANY:
        proto_str = "any";
        break;
      default:
        snprintf(proto_buf, sizeof(proto_buf), "%u", (unsigned)rule->protocol);
        proto_str = proto_buf;
        break;
    }

    LOG(prefix, "ACL #%u: %s %s %s %s", (unsigned)rule->rule_id, action, rule->src_cidr,
        rule->dst_cidr, proto_str);
  }
}
