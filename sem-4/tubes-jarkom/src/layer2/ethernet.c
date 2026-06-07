#define _POSIX_C_SOURCE 200809L

#include "ethernet.h"

#include "utils/byteops.h"
#include "utils/magi_error.h"

#include <stdlib.h>
#include <string.h>

/**
 * Check if a MAC address is the broadcast address (FF:FF:FF:FF:FF:FF).
 *
 * Compares all 6 bytes against 0xFF. Returns false if the pointer is NULL.
 *
 * @param mac Pointer to a 6-byte MAC address.
 * @return true if all 6 bytes are 0xFF, false otherwise.
 */
bool ethernet_mac_is_broadcast(const uint8_t mac[ETHERNET_MAC_LEN]) {
  if (mac == NULL) {
    return false;
  }

  for (size_t index = 0U; index < ETHERNET_MAC_LEN; ++index) {
    if (mac[index] != 0xFFU) {
      return false;
    }
  }

  return true;
}

/**
 * Check if a MAC address is a multicast address by examining the I/G bit.
 *
 * The least-significant bit of the first byte (I/G bit) indicates multicast
 * when set to 1. Returns false if the pointer is NULL.
 *
 * @param mac Pointer to a 6-byte MAC address.
 * @return true if the I/G bit is set, false otherwise.
 */
bool ethernet_mac_is_multicast(const uint8_t mac[ETHERNET_MAC_LEN]) {
  return mac != NULL && (mac[0] & 0x01U) != 0U;
}

/**
 * Compare two MAC addresses for equality.
 *
 * Performs a byte-by-byte comparison using memcmp. Returns false if either
 * pointer is NULL.
 *
 * @param lhs Pointer to the first 6-byte MAC address.
 * @param rhs Pointer to the second 6-byte MAC address.
 * @return true if both addresses are identical, false otherwise.
 */
bool ethernet_mac_equal(const uint8_t lhs[ETHERNET_MAC_LEN], const uint8_t rhs[ETHERNET_MAC_LEN]) {
  return lhs != NULL && rhs != NULL && memcmp(lhs, rhs, ETHERNET_MAC_LEN) == 0;
}

/**
 * Fill a 6-byte buffer with the Ethernet broadcast address (FF:FF:FF:FF:FF:FF).
 *
 * Does nothing if the output pointer is NULL.
 *
 * @param out Pointer to a 6-byte buffer to fill with 0xFF.
 */
void ethernet_mac_broadcast(uint8_t out[ETHERNET_MAC_LEN]) {
  if (out != NULL) {
    memset(out, 0xFF, ETHERNET_MAC_LEN);
  }
}

/**
 * Parse a raw Ethernet frame from a byte buffer into an EthernetFrame struct.
 *
 * Supports both untagged (14-byte header) and 802.1Q VLAN-tagged (18-byte
 * header) frames. Sets the payload pointer to point into the input buffer;
 * the caller must ensure the buffer remains valid while the frame is in use.
 *
 * @param data Pointer to the raw frame bytes.
 * @param len  Length of the raw frame in bytes.
 * @param frame_out Pointer to an EthernetFrame struct to populate.
 * @return MAGI_OK on success, or MAGI_ERR_BADARGS if the data is too short
 *         or invalid.
 */
int ethernet_frame_from_bytes(const uint8_t* data, size_t len, EthernetFrame* frame_out) {
  if (data == NULL || frame_out == NULL || len < ETHERNET_MIN_FRAME_LEN) {
    magi_errno = MAGI_ERR_BADARGS;
    return MAGI_ERR_BADARGS;
  }

  memset(frame_out, 0, sizeof(*frame_out));
  memcpy(frame_out->dst_mac, data, ETHERNET_MAC_LEN);
  memcpy(frame_out->src_mac, data + ETHERNET_MAC_LEN, ETHERNET_MAC_LEN);

  uint16_t ethertype = READ_U16(data, 12U);
  if (ethertype == ETHERNET_TYPE_VLAN) {
    if (len < ETHERNET_VLAN_FRAME_LEN) {
      magi_errno = MAGI_ERR_BADARGS;
      return MAGI_ERR_BADARGS;
    }

    uint16_t tci = READ_U16(data, 14U);
    frame_out->vlan_present = true;
    frame_out->vlan_id = (uint16_t)(tci & 0x0FFFU);
    frame_out->ethertype = READ_U16(data, 16U);
    frame_out->payload = data + ETHERNET_VLAN_FRAME_LEN;
    frame_out->payload_len = len - ETHERNET_VLAN_FRAME_LEN;
    return MAGI_OK;
  }

  frame_out->ethertype = ethertype;
  frame_out->payload = data + ETHERNET_MIN_FRAME_LEN;
  frame_out->payload_len = len - ETHERNET_MIN_FRAME_LEN;
  return MAGI_OK;
}

/**
 * Serialize an EthernetFrame struct into a heap-allocated byte buffer.
 *
 * Allocates memory for the serialized frame; the caller is responsible for
 * freeing the output buffer with free(). Supports both untagged and 802.1Q
 * VLAN-tagged output.
 *
 * @param frame     Pointer to the EthernetFrame to serialize.
 * @param bytes_out Output pointer for the allocated byte buffer.
 * @param len_out   Output pointer for the length of the allocated buffer.
 * @return MAGI_OK on success, MAGI_ERR_BADARGS if arguments are invalid, or
 *         MAGI_ERR_NOMEM on allocation failure.
 */
int ethernet_frame_to_bytes(const EthernetFrame* frame, uint8_t** bytes_out, size_t* len_out) {
  if (frame == NULL || bytes_out == NULL || len_out == NULL ||
      (frame->payload_len > 0U && frame->payload == NULL)) {
    magi_errno = MAGI_ERR_BADARGS;
    return MAGI_ERR_BADARGS;
  }

  size_t header_len = frame->vlan_present ? ETHERNET_VLAN_FRAME_LEN : ETHERNET_MIN_FRAME_LEN;
  size_t total_len = header_len + frame->payload_len;
  uint8_t* bytes = malloc(total_len);
  if (bytes == NULL) {
    magi_errno = MAGI_ERR_NOMEM;
    return MAGI_ERR_NOMEM;
  }

  memcpy(bytes, frame->dst_mac, ETHERNET_MAC_LEN);
  memcpy(bytes + ETHERNET_MAC_LEN, frame->src_mac, ETHERNET_MAC_LEN);

  if (frame->vlan_present) {
    WRITE_U16(bytes, 12U, ETHERNET_TYPE_VLAN);
    WRITE_U16(bytes, 14U, frame->vlan_id & 0x0FFFU);
    WRITE_U16(bytes, 16U, frame->ethertype);
  } else {
    WRITE_U16(bytes, 12U, frame->ethertype);
  }

  if (frame->payload_len > 0U) {
    memcpy(bytes + header_len, frame->payload, frame->payload_len);
  }

  *bytes_out = bytes;
  *len_out = total_len;
  return MAGI_OK;
}
