#define _POSIX_C_SOURCE 200809L

#include "arp.h"

#include "layer2/ethernet.h"
#include "utils/byteops.h"
#include "utils/magi_error.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/**
 * Parse an IPv4 address from a dotted-decimal or CIDR string into 4 bytes.
 *
 * Accepts formats such as "192.168.1.1" or "192.168.1.1/24". In the CIDR
 * case, only the address portion is parsed; the prefix length is ignored.
 * Each octet must be in the range 0-255.
 *
 * @param text Null-terminated input string.
 * @param out  Output array of 4 bytes to receive the parsed address.
 * @return MAGI_OK on success, or MAGI_ERR_BADARGS if parsing fails or the
 *         pointer is NULL.
 */
int arp_ipv4_from_string(const char* text, uint8_t out[4]) {
  if (text == NULL || out == NULL) {
    magi_errno = MAGI_ERR_BADARGS;
    return MAGI_ERR_BADARGS;
  }

  unsigned int octets[4] = {0U};
  char tail = '\0';
  int parsed =
      sscanf(text, "%3u.%3u.%3u.%3u%c", &octets[0], &octets[1], &octets[2], &octets[3], &tail);

  if (parsed < 4 || (parsed == 5 && tail != '/') || octets[0] > 255U || octets[1] > 255U ||
      octets[2] > 255U || octets[3] > 255U) {
    magi_errno = MAGI_ERR_BADARGS;
    return MAGI_ERR_BADARGS;
  }

  for (size_t index = 0U; index < 4U; ++index) {
    out[index] = (uint8_t)octets[index];
  }

  return MAGI_OK;
}

/**
 * Format an IPv4 address as a dotted-decimal string.
 *
 * Writes a null-terminated string of the form "192.168.1.1" into the output
 * buffer. If the IP pointer is NULL, the output buffer is set to an empty
 * string. If the output pointer is NULL, this function does nothing.
 *
 * @param ip  4-byte IPv4 address to format.
 * @param out Output buffer (must be at least 16 bytes).
 */
void arp_ipv4_to_string(const uint8_t ip[4], char out[16]) {
  if (out == NULL) {
    return;
  }

  if (ip == NULL) {
    out[0] = '\0';
    return;
  }

  snprintf(out, 16U, "%u.%u.%u.%u", (unsigned)ip[0], (unsigned)ip[1], (unsigned)ip[2],
           (unsigned)ip[3]);
}

int arp_message_from_bytes(const uint8_t* data, size_t len, ARPMessage* message_out) {
  if (data == NULL || message_out == NULL || len < ARP_ETHERNET_IPV4_LEN) {
    magi_errno = MAGI_ERR_BADARGS;
    return MAGI_ERR_BADARGS;
  }

  uint16_t htype = READ_U16(data, 0U);
  uint16_t ptype = READ_U16(data, 2U);
  uint8_t hlen = data[4];
  uint8_t plen = data[5];

  if (htype != 1U || ptype != ETHERNET_TYPE_IPV4 || hlen != ETHERNET_MAC_LEN || plen != 4U) {
    magi_errno = MAGI_ERR_BADARGS;
    return MAGI_ERR_BADARGS;
  }

  memset(message_out, 0, sizeof(*message_out));
  message_out->opcode = READ_U16(data, 6U);
  memcpy(message_out->sender_mac, data + 8U, ETHERNET_MAC_LEN);
  memcpy(message_out->sender_ip, data + 14U, 4U);
  memcpy(message_out->target_mac, data + 18U, ETHERNET_MAC_LEN);
  memcpy(message_out->target_ip, data + 24U, 4U);
  return MAGI_OK;
}

int arp_message_to_bytes(const ARPMessage* message, uint8_t** bytes_out, size_t* len_out) {
  if (message == NULL || bytes_out == NULL || len_out == NULL) {
    magi_errno = MAGI_ERR_BADARGS;
    return MAGI_ERR_BADARGS;
  }

  uint8_t* bytes = calloc(ARP_ETHERNET_IPV4_LEN, 1U);
  if (bytes == NULL) {
    magi_errno = MAGI_ERR_NOMEM;
    return MAGI_ERR_NOMEM;
  }

  WRITE_U16(bytes, 0U, 1U);
  WRITE_U16(bytes, 2U, ETHERNET_TYPE_IPV4);
  bytes[4] = ETHERNET_MAC_LEN;
  bytes[5] = 4U;
  WRITE_U16(bytes, 6U, message->opcode);
  memcpy(bytes + 8U, message->sender_mac, ETHERNET_MAC_LEN);
  memcpy(bytes + 14U, message->sender_ip, 4U);
  memcpy(bytes + 18U, message->target_mac, ETHERNET_MAC_LEN);
  memcpy(bytes + 24U, message->target_ip, 4U);

  *bytes_out = bytes;
  *len_out = ARP_ETHERNET_IPV4_LEN;
  return MAGI_OK;
}
