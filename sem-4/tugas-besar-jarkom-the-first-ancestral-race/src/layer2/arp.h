/**
 * @file arp.h
 * @brief ARP packet serialization helpers for Ethernet/IPv4.
 */

#ifndef MAGI_LAYER2_ARP_H
#define MAGI_LAYER2_ARP_H

#include <stddef.h>
#include <stdint.h>

#define ARP_ETHERNET_IPV4_LEN 28U
#define ARP_OPCODE_REQUEST 1U
#define ARP_OPCODE_REPLY 2U

/**
 * @brief Parsed ARP message for Ethernet/IPv4.
 */
typedef struct ARPMessage {
  uint16_t opcode;
  uint8_t sender_mac[6];
  uint8_t sender_ip[4];
  uint8_t target_mac[6];
  uint8_t target_ip[4];
} ARPMessage;

/**
 * @brief Parse dotted IPv4 text, accepting an optional CIDR suffix.
 *
 * @param text Dotted IPv4 string, e.g. 192.168.1.10 or 192.168.1.10/24.
 * @param out Destination 4-byte address.
 * @return MAGI_OK on success, otherwise an error code.
 */
int arp_ipv4_from_string(const char* text, uint8_t out[4]);

/**
 * @brief Format IPv4 bytes as dotted text.
 *
 * @param ip Input 4-byte address.
 * @param out Destination buffer with room for 16 bytes including NUL.
 */
void arp_ipv4_to_string(const uint8_t ip[4], char out[16]);

/**
 * @brief Parse raw bytes into an ARPMessage.
 *
 * @param data Raw ARP payload.
 * @param len Raw payload length.
 * @param message_out Destination parsed message.
 * @return MAGI_OK on success, otherwise an error code.
 */
int arp_message_from_bytes(const uint8_t* data, size_t len, ARPMessage* message_out);

/**
 * @brief Serialize an ARPMessage into a heap buffer.
 *
 * @param message Source message.
 * @param bytes_out Owned output buffer.
 * @param len_out Output length.
 * @return MAGI_OK on success, otherwise an error code.
 */
int arp_message_to_bytes(const ARPMessage* message, uint8_t** bytes_out, size_t* len_out);

#endif
