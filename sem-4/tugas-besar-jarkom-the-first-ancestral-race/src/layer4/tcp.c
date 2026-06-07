/**
 * @file tcp.c
 * @brief TCP segment serialization and parsing.
 */

#define _POSIX_C_SOURCE 200809L

#include "tcp.h"

#include "utils/byteops.h"
#include "utils/magi_error.h"

#include <stdlib.h>
#include <string.h>

/**
 * @brief Serialize a TCPSegment into a byte buffer.
 *
 * Builds the 20-byte TCP header in network byte order, computes the TCP
 * checksum over the segment using the IPv4 pseudo-header (src_ip, dst_ip,
 * protocol=6, segment length), and copies the payload after the header.
 * On success the segment's checksum and data_offset fields are updated.
 *
 * @param seg     Segment to serialize.
 * @param src_ip  Source IPv4 address (4 bytes) for pseudo-header.
 * @param dst_ip  Destination IPv4 address (4 bytes) for pseudo-header.
 * @param out     Output buffer (must hold at least TCP_HEADER_LEN + payload_len bytes).
 * @param out_len Capacity of the output buffer.
 * @return MAGI_OK on success, MAGI_ERR_BADARGS on null pointer or insufficient buffer.
 */
int tcp_pack(TCPSegment* seg, const uint8_t src_ip[4], const uint8_t dst_ip[4], uint8_t* out,
             size_t out_len) {
  if (seg == NULL || src_ip == NULL || dst_ip == NULL || out == NULL) {
    magi_errno = MAGI_ERR_BADARGS;
    return MAGI_ERR_BADARGS;
  }

  size_t total_len = TCP_HEADER_LEN + seg->payload_len;
  if (out_len < total_len) {
    magi_errno = MAGI_ERR_BADARGS;
    return MAGI_ERR_BADARGS;
  }

  memset(out, 0, TCP_HEADER_LEN);

  WRITE_U16(out, 0U, seg->src_port);
  WRITE_U16(out, 2U, seg->dst_port);
  WRITE_U32(out, 4U, seg->seq_num);
  WRITE_U32(out, 8U, seg->ack_num);

  uint8_t data_offset = (seg->data_offset != 0U) ? seg->data_offset : TCP_DATA_OFFSET_DEFAULT;
  WRITE_U8(out, 12U, (data_offset << 4U) & 0xF0U);
  WRITE_U8(out, 13U, seg->flags);
  WRITE_U16(out, 14U, seg->window_size != 0U ? seg->window_size : TCP_WINDOW_SIZE_DEFAULT);
  WRITE_U16(out, 16U, 0U); /* checksum placeholder */
  WRITE_U16(out, 18U, 0U); /* urgent pointer */

  if (seg->payload_len > 0U && seg->payload != NULL) {
    memcpy(out + TCP_HEADER_LEN, seg->payload, seg->payload_len);
  }

  /* Build pseudo-header for checksum */
  uint8_t pseudo_hdr[12];
  memcpy(pseudo_hdr, src_ip, 4U);
  memcpy(pseudo_hdr + 4U, dst_ip, 4U);
  pseudo_hdr[8] = 0U;
  pseudo_hdr[9] = 6U; /* IPPROTO_TCP */
  WRITE_U16(pseudo_hdr, 10U, (uint16_t)total_len);

  uint16_t cksum = transport_checksum(pseudo_hdr, 12U, out, total_len);
  WRITE_U16(out, 16U, cksum);

  /* Update segment fields */
  seg->checksum = cksum;
  seg->data_offset = data_offset;

  return MAGI_OK;
}

/**
 * @brief Deserialize a byte buffer into a TCPSegment.
 *
 * Parses the 20-byte TCP header from the input buffer, validates the
 * checksum using the IPv4 pseudo-header (src_ip, dst_ip, protocol=6,
 * segment length), and sets the payload pointer into the input buffer.
 * The segment is zeroed before parsing.
 *
 * @param seg     Segment to populate (output).
 * @param src_ip  Source IPv4 address (4 bytes) for checksum validation.
 * @param dst_ip  Destination IPv4 address (4 bytes) for checksum validation.
 * @param in      Input byte buffer containing the serialized TCP segment.
 * @param in_len  Length of the input buffer.
 * @return MAGI_OK on success, MAGI_ERR_BADCKSUM if the checksum is invalid,
 *         MAGI_ERR_BADARGS on null pointer or malformed header.
 */
int tcp_unpack(TCPSegment* seg, const uint8_t src_ip[4], const uint8_t dst_ip[4], const uint8_t* in,
               size_t in_len) {
  if (seg == NULL || src_ip == NULL || dst_ip == NULL || in == NULL) {
    magi_errno = MAGI_ERR_BADARGS;
    return MAGI_ERR_BADARGS;
  }

  if (in_len < TCP_HEADER_LEN) {
    magi_errno = MAGI_ERR_BADARGS;
    return MAGI_ERR_BADARGS;
  }

  memset(seg, 0, sizeof(*seg));

  seg->src_port = READ_U16(in, 0U);
  seg->dst_port = READ_U16(in, 2U);
  seg->seq_num = READ_U32(in, 4U);
  seg->ack_num = READ_U32(in, 8U);

  uint8_t offset_byte = in[12U];
  seg->data_offset = (uint8_t)((offset_byte >> 4U) & 0x0FU);
  seg->flags = in[13U];
  seg->window_size = READ_U16(in, 14U);
  seg->checksum = READ_U16(in, 16U);

  size_t hdr_len = (size_t)(seg->data_offset) * 4U;
  if (hdr_len < TCP_HEADER_LEN || hdr_len > in_len) {
    magi_errno = MAGI_ERR_BADARGS;
    return MAGI_ERR_BADARGS;
  }

  /* Verify checksum */
  uint8_t pseudo_hdr[12];
  memcpy(pseudo_hdr, src_ip, 4U);
  memcpy(pseudo_hdr + 4U, dst_ip, 4U);
  pseudo_hdr[8] = 0U;
  pseudo_hdr[9] = 6U; /* IPPROTO_TCP */
  WRITE_U16(pseudo_hdr, 10U, (uint16_t)in_len);

  if (transport_checksum(pseudo_hdr, 12U, in, in_len) != 0U) {
    magi_errno = MAGI_ERR_BADCKSUM;
    return MAGI_ERR_BADCKSUM;
  }

  seg->payload = in + hdr_len;
  seg->payload_len = in_len - hdr_len;

  return MAGI_OK;
}
