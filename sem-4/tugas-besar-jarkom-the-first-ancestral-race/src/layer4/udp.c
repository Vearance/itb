/**
 * @file udp.c
 * @brief UDP datagram serialization and parsing.
 */

#define _POSIX_C_SOURCE 200809L

#include "udp.h"

#include "utils/byteops.h"
#include "utils/magi_error.h"

#include <stdlib.h>
#include <string.h>

/**
 * @brief Serialize a UDPDatagram into a byte buffer.
 *
 * Builds the 8-byte UDP header in network byte order, computes the UDP
 * checksum using the IPv4 pseudo-header (src_ip, dst_ip, protocol=17,
 * datagram length), and copies the payload after the header.
 * Per RFC 768, a checksum of 0x0000 is stored as 0xFFFF.
 * On success the datagram's checksum and length fields are updated.
 *
 * @param dgram   Datagram to serialize.
 * @param src_ip  Source IPv4 address (4 bytes) for pseudo-header.
 * @param dst_ip  Destination IPv4 address (4 bytes) for pseudo-header.
 * @param out     Output buffer (must hold at least UDP_HEADER_LEN + payload_len bytes).
 * @param out_len Capacity of the output buffer.
 * @return MAGI_OK on success, MAGI_ERR_BADARGS on null pointer or if
 *         total length exceeds 0xFFFF.
 */
int udp_pack(UDPDatagram* dgram, const uint8_t src_ip[4], const uint8_t dst_ip[4], uint8_t* out,
             size_t out_len) {
  if (dgram == NULL || src_ip == NULL || dst_ip == NULL || out == NULL) {
    magi_errno = MAGI_ERR_BADARGS;
    return MAGI_ERR_BADARGS;
  }

  size_t total_len = UDP_HEADER_LEN + dgram->payload_len;
  if (out_len < total_len || total_len > 0xFFFFU) {
    magi_errno = MAGI_ERR_BADARGS;
    return MAGI_ERR_BADARGS;
  }

  memset(out, 0, UDP_HEADER_LEN);

  WRITE_U16(out, 0U, dgram->src_port);
  WRITE_U16(out, 2U, dgram->dst_port);
  WRITE_U16(out, 4U, (uint16_t)total_len);
  WRITE_U16(out, 6U, 0U); /* checksum placeholder */

  if (dgram->payload_len > 0U && dgram->payload != NULL) {
    memcpy(out + UDP_HEADER_LEN, dgram->payload, dgram->payload_len);
  }

  /* Build pseudo-header for checksum */
  uint8_t pseudo_hdr[12];
  memcpy(pseudo_hdr, src_ip, 4U);
  memcpy(pseudo_hdr + 4U, dst_ip, 4U);
  pseudo_hdr[8] = 0U;
  pseudo_hdr[9] = 17U; /* IPPROTO_UDP */
  WRITE_U16(pseudo_hdr, 10U, (uint16_t)total_len);

  uint16_t cksum = transport_checksum(pseudo_hdr, 12U, out, total_len);

  /* RFC 768: if checksum computes to 0x0000, store as 0xFFFF */
  if (cksum == 0U) {
    cksum = 0xFFFFU;
  }

  WRITE_U16(out, 6U, cksum);

  dgram->checksum = cksum;
  dgram->length = (uint16_t)total_len;

  return MAGI_OK;
}

/**
 * @brief Deserialize a byte buffer into a UDPDatagram.
 *
 * Parses the 8-byte UDP header from the input buffer. If the stored
 * checksum is non-zero, validates it using the IPv4 pseudo-header
 * (src_ip, dst_ip, protocol=17, datagram length). A zero checksum
 * indicates the sender did not compute one (RFC 768).
 * Sets the payload pointer into the input buffer.
 *
 * @param dgram   Datagram to populate (output).
 * @param src_ip  Source IPv4 address (4 bytes) for optional checksum validation.
 * @param dst_ip  Destination IPv4 address (4 bytes) for optional checksum validation.
 * @param in      Input byte buffer containing the serialized UDP datagram.
 * @param in_len  Length of the input buffer.
 * @return MAGI_OK on success, MAGI_ERR_BADCKSUM if checksum validation fails,
 *         MAGI_ERR_BADARGS on null pointer or malformed header.
 */
int udp_unpack(UDPDatagram* dgram, const uint8_t src_ip[4], const uint8_t dst_ip[4],
               const uint8_t* in, size_t in_len) {
  if (dgram == NULL || src_ip == NULL || dst_ip == NULL || in == NULL) {
    magi_errno = MAGI_ERR_BADARGS;
    return MAGI_ERR_BADARGS;
  }

  if (in_len < UDP_HEADER_LEN) {
    magi_errno = MAGI_ERR_BADARGS;
    return MAGI_ERR_BADARGS;
  }

  memset(dgram, 0, sizeof(*dgram));

  dgram->src_port = READ_U16(in, 0U);
  dgram->dst_port = READ_U16(in, 2U);
  dgram->length = READ_U16(in, 4U);
  dgram->checksum = READ_U16(in, 6U);

  if (dgram->length < UDP_HEADER_LEN || dgram->length > in_len) {
    magi_errno = MAGI_ERR_BADARGS;
    return MAGI_ERR_BADARGS;
  }

  /* Verify checksum if it is non-zero (zero means unused per RFC 768) */
  if (dgram->checksum != 0U) {
    uint8_t pseudo_hdr[12];
    memcpy(pseudo_hdr, src_ip, 4U);
    memcpy(pseudo_hdr + 4U, dst_ip, 4U);
    pseudo_hdr[8] = 0U;
    pseudo_hdr[9] = 17U; /* IPPROTO_UDP */
    WRITE_U16(pseudo_hdr, 10U, dgram->length);

    if (transport_checksum(pseudo_hdr, 12U, in, dgram->length) != 0U) {
      magi_errno = MAGI_ERR_BADCKSUM;
      return MAGI_ERR_BADCKSUM;
    }
  }

  dgram->payload = in + UDP_HEADER_LEN;
  dgram->payload_len = (size_t)dgram->length - UDP_HEADER_LEN;

  return MAGI_OK;
}
