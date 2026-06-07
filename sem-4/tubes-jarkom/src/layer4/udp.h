/**
 * @file udp.h
 * @brief UDP datagram serialization and parsing.
 */

#ifndef MAGI_LAYER4_UDP_H
#define MAGI_LAYER4_UDP_H

#include <stddef.h>
#include <stdint.h>

#include "core/packet.h"

#define UDP_HEADER_LEN 8U

/**
 * @brief Parsed UDP datagram.
 */
typedef struct UDPDatagram {
  Packet base;
  uint16_t src_port;
  uint16_t dst_port;
  uint16_t length; /* 8 + payload_len */
  uint16_t checksum;
  const uint8_t* payload;
  size_t payload_len;
} UDPDatagram;

/**
 * @brief Serialize a UDP datagram.
 *
 * Checksum is computed using the IPv4 pseudo-header.
 * If checksum computes to 0x0000, it is stored as 0xFFFF (per RFC 768).
 *
 * @param dgram   Source datagram.
 * @param src_ip  Source IPv4 address for pseudo-header.
 * @param dst_ip  Destination IPv4 address for pseudo-header.
 * @param out     Output buffer (must be at least UDP_HEADER_LEN + payload_len).
 * @param out_len Output buffer size.
 * @return MAGI_OK on success, otherwise an error code.
 */
int udp_pack(UDPDatagram* dgram, const uint8_t src_ip[4], const uint8_t dst_ip[4], uint8_t* out,
             size_t out_len);

/**
 * @brief Parse a UDP datagram from raw bytes.
 *
 * @param dgram   Destination datagram struct.
 * @param src_ip  Source IPv4 address for pseudo-header.
 * @param dst_ip  Destination IPv4 address for pseudo-header.
 * @param in      Raw bytes.
 * @param in_len  Raw bytes length.
 * @return MAGI_OK on success, MAGI_ERR_BADCKSUM on checksum mismatch.
 */
int udp_unpack(UDPDatagram* dgram, const uint8_t src_ip[4], const uint8_t dst_ip[4],
               const uint8_t* in, size_t in_len);

#endif /* MAGI_LAYER4_UDP_H */
