/**
 * @file mac.h
 * @brief MAC address helpers for generation and parsing.
 */

#ifndef MAGI_UTILS_MAC_H
#define MAGI_UTILS_MAC_H

#include <stdint.h>

/**
 * @brief Deterministically generate a locally-administered unicast MAC.
 *
 * @param out Destination 6-byte buffer.
 * @param node_name Node name used as deterministic seed input.
 * @param port Port number used as deterministic seed input.
 */
void mac_generate(uint8_t out[6], const char* node_name, uint16_t port);

/**
 * @brief Format a MAC address as XX:XX:XX:XX:XX:XX.
 *
 * @param mac Input 6-byte MAC address.
 * @param out Destination buffer with room for 18 bytes including NUL.
 */
void mac_to_str(const uint8_t mac[6], char out[18]);

/**
 * @brief Parse a textual MAC address into 6 bytes.
 *
 * @param str Input text in XX:XX:XX:XX:XX:XX form.
 * @param out Destination 6-byte buffer.
 * @return MAGI_OK on success, otherwise an error code.
 */
int mac_from_str(const char* str, uint8_t out[6]);

#endif