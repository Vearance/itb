#define _POSIX_C_SOURCE 200809L

#include "icmp.h"

#include "utils/byteops.h"
#include "utils/magi_error.h"

#include <stdlib.h>
#include <string.h>

int icmp_pack(ICMPMessage* msg, uint8_t* out, size_t out_len) {
  if (msg == NULL || out == NULL || out_len < ICMP_HEADER_LEN ||
      (msg->payload_len > 0U && msg->payload == NULL)) {
    magi_errno = MAGI_ERR_BADARGS;
    return MAGI_ERR_BADARGS;
  }

  size_t total_len = ICMP_HEADER_LEN + msg->payload_len;
  if (out_len < total_len) {
    magi_errno = MAGI_ERR_BADARGS;
    return MAGI_ERR_BADARGS;
  }

  memset(out, 0, total_len);
  WRITE_U8(out, 0U, msg->type);
  WRITE_U8(out, 1U, msg->code);
  WRITE_U16(out, 2U, 0U);

  if (msg->type == ICMP_TYPE_ECHO_REQUEST || msg->type == ICMP_TYPE_ECHO_REPLY) {
    WRITE_U16(out, 4U, msg->identifier);
    WRITE_U16(out, 6U, msg->sequence);
  }

  if (msg->payload_len > 0U) {
    memcpy(out + ICMP_HEADER_LEN, msg->payload, msg->payload_len);
  }

  msg->checksum = ipv4_checksum(out, total_len);
  WRITE_U16(out, 2U, msg->checksum);
  return MAGI_OK;
}

int icmp_unpack(ICMPMessage* msg, const uint8_t* in, size_t in_len) {
  if (msg == NULL || in == NULL || in_len < ICMP_HEADER_LEN) {
    magi_errno = MAGI_ERR_BADARGS;
    return MAGI_ERR_BADARGS;
  }

  if (ipv4_checksum(in, in_len) != 0U) {
    magi_errno = MAGI_ERR_BADCKSUM;
    return MAGI_ERR_BADCKSUM;
  }

  memset(msg, 0, sizeof(*msg));
  msg->type = in[0];
  msg->code = in[1];
  msg->checksum = READ_U16(in, 2U);
  msg->identifier = READ_U16(in, 4U);
  msg->sequence = READ_U16(in, 6U);
  msg->payload = in + ICMP_HEADER_LEN;
  msg->payload_len = in_len - ICMP_HEADER_LEN;
  return MAGI_OK;
}

int icmp_message_to_bytes(ICMPMessage* msg, uint8_t** bytes_out, size_t* len_out) {
  if (msg == NULL || bytes_out == NULL || len_out == NULL) {
    magi_errno = MAGI_ERR_BADARGS;
    return MAGI_ERR_BADARGS;
  }

  size_t total_len = ICMP_HEADER_LEN + msg->payload_len;
  uint8_t* bytes = malloc(total_len);
  if (bytes == NULL) {
    magi_errno = MAGI_ERR_NOMEM;
    return MAGI_ERR_NOMEM;
  }

  int status = icmp_pack(msg, bytes, total_len);
  if (status != MAGI_OK) {
    free(bytes);
    return status;
  }

  *bytes_out = bytes;
  *len_out = total_len;
  return MAGI_OK;
}
