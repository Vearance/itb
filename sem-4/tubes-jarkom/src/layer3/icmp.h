/**
 * @file icmp.h
 * @brief ICMP message serialization helpers.
 */

#ifndef MAGI_LAYER3_ICMP_H
#define MAGI_LAYER3_ICMP_H

#include <stddef.h>
#include <stdint.h>

#include "core/packet.h"

#define ICMP_TYPE_ECHO_REPLY 0U
#define ICMP_TYPE_DEST_UNREACHABLE 3U
#define ICMP_TYPE_ECHO_REQUEST 8U
#define ICMP_TYPE_TIME_EXCEEDED 11U
#define ICMP_HEADER_LEN 8U

typedef struct ICMPMessage {
  Packet base;
  uint8_t type;
  uint8_t code;
  uint16_t checksum;
  uint16_t identifier;
  uint16_t sequence;
  const uint8_t* payload;
  size_t payload_len;
} ICMPMessage;

int icmp_pack(ICMPMessage* msg, uint8_t* out, size_t out_len);
int icmp_unpack(ICMPMessage* msg, const uint8_t* in, size_t in_len);
int icmp_message_to_bytes(ICMPMessage* msg, uint8_t** bytes_out, size_t* len_out);

#endif
