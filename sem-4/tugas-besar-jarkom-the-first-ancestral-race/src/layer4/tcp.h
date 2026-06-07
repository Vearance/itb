/**
 * @file tcp.h
 * @brief TCP segment serialization and parsing.
 */

#ifndef MAGI_LAYER4_TCP_H
#define MAGI_LAYER4_TCP_H

#include <stddef.h>
#include <stdint.h>

#include "core/packet.h"

#define TCP_HEADER_LEN 20U
#define TCP_DATA_OFFSET_DEFAULT 5U
#define TCP_WINDOW_SIZE_DEFAULT 65535U

/* TCP flag bits */
#define TCP_FLAG_FIN 0x01U
#define TCP_FLAG_SYN 0x02U
#define TCP_FLAG_RST 0x04U
#define TCP_FLAG_PSH 0x08U
#define TCP_FLAG_ACK 0x10U
#define TCP_FLAG_SYNACK (TCP_FLAG_SYN | TCP_FLAG_ACK)
#define TCP_FLAG_FINACK (TCP_FLAG_FIN | TCP_FLAG_ACK)

/**
 * @brief Parsed TCP segment.
 */
typedef struct TCPSegment {
  Packet base;
  uint16_t src_port;
  uint16_t dst_port;
  uint32_t seq_num;
  uint32_t ack_num;
  uint8_t data_offset; /* 4-bit field; always 5 for our impl */
  uint8_t flags;
  uint16_t window_size;
  uint16_t checksum;
  const uint8_t* payload;
  size_t payload_len;
} TCPSegment;

/**
 * @brief Serialize a TCP segment into a byte buffer.
 *
 * Checksum is computed using the IPv4 pseudo-header.
 *
 * @param seg     Source segment.
 * @param src_ip  Source IPv4 address (4 bytes) for pseudo-header.
 * @param dst_ip  Destination IPv4 address (4 bytes) for pseudo-header.
 * @param out     Output buffer (must be at least TCP_HEADER_LEN + payload_len).
 * @param out_len Output buffer size.
 * @return MAGI_OK on success, otherwise an error code.
 */
int tcp_pack(TCPSegment* seg, const uint8_t src_ip[4], const uint8_t dst_ip[4], uint8_t* out,
             size_t out_len);

/**
 * @brief Parse a TCP segment from raw bytes.
 *
 * Validates the checksum using the IPv4 pseudo-header.
 *
 * @param seg     Destination segment struct.
 * @param src_ip  Source IPv4 address (4 bytes) for pseudo-header.
 * @param dst_ip  Destination IPv4 address (4 bytes) for pseudo-header.
 * @param in      Raw bytes.
 * @param in_len  Raw bytes length.
 * @return MAGI_OK on success, MAGI_ERR_BADCKSUM on checksum mismatch.
 */
int tcp_unpack(TCPSegment* seg, const uint8_t src_ip[4], const uint8_t dst_ip[4], const uint8_t* in,
               size_t in_len);

#endif /* MAGI_LAYER4_TCP_H */
