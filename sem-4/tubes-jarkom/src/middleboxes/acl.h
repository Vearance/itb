/**
 * @file acl.h
 * @brief Access Control List — IP packet filtering on routers.
 *
 * ACL rules are evaluated in order. The first matching rule wins.
 * If no rule matches, the default action is PERMIT.
 */

#ifndef MAGI_MIDDLEBOXES_ACL_H
#define MAGI_MIDDLEBOXES_ACL_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

struct IPv4Packet;

#define ACL_CIDR_ANY "any"
#define ACL_CIDR_LEN 20U
#define ACL_PROTO_ANY 0U
#define ACL_PORT_ANY 0U

/**
 * @brief One ACL rule.
 */
typedef struct ACLRule {
  char src_cidr[ACL_CIDR_LEN]; /**< Source CIDR (e.g. "192.168.1.0/24") or "any". */
  char dst_cidr[ACL_CIDR_LEN]; /**< Destination CIDR or "any". */
  uint8_t protocol;             /**< 0=any, 1=ICMP, 6=TCP, 17=UDP. */
  uint16_t src_port;            /**< 0=any. */
  uint16_t dst_port;            /**< 0=any. */
  bool permit;                  /**< true=allow, false=deny. */
  uint32_t rule_id;             /**< Rule index for logging. */
} ACLRule;

/**
 * @brief Ordered ACL rule table.
 */
typedef struct ACLTable {
  ACLRule* rules;
  size_t count;
  size_t capacity;
} ACLTable;

/**
 * @brief Allocate and initialise an empty ACL table.
 *
 * @return New ACLTable, or NULL on allocation failure.
 */
ACLTable* acl_table_new(void);

/**
 * @brief Free an ACL table and all its rules.
 *
 * @param t ACL table to free. NULL is allowed.
 */
void acl_table_free(ACLTable* t);

/**
 * @brief Append a rule to the end of the ACL table.
 *
 * The first matching rule wins, so order matters.
 *
 * @param t    ACL table.
 * @param rule Rule to append (copied internally).
 * @return MAGI_OK on success, otherwise an error code.
 */
int acl_add_rule(ACLTable* t, ACLRule rule);

/**
 * @brief Check an IPv4 packet against the ACL table.
 *
 * Evaluates rules in order. The first matching rule's action is returned.
 * If no rule matches, the default action is PERMIT (MAGI_OK).
 *
 * @param t   ACL table. If NULL, all packets pass.
 * @param pkt Parsed IPv4 packet to check.
 * @return MAGI_OK if permitted, MAGI_ERR_ACL_DENY if denied.
 */
int acl_check(const ACLTable* t, const struct IPv4Packet* pkt);

/**
 * @brief Parse a protocol name or number from a string.
 *
 * Accepts: "any", "icmp", "tcp", "udp", or a numeric protocol number (0-255).
 * Returns 0xFF on failure.
 *
 * @param text Input string.
 * @return Protocol number, ACL_PROTO_ANY, or 0xFF on failure.
 */
uint8_t acl_parse_protocol(const char* text);

/**
 * @brief Remove all rules from an ACL table.
 *
 * @param t ACL table to clear. NULL is allowed.
 */
void acl_clear(ACLTable* t);

/**
 * @brief Print all rules in an ACL table via LOG().
 *
 * @param t   ACL table to display.
 * @param name Router name for log prefix.
 */
void acl_print(const ACLTable* t, const char* name);

#endif /* MAGI_MIDDLEBOXES_ACL_H */
