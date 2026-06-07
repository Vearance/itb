/**
 * @file byteops.h
 * @brief Endian-safe byte helpers and checksum utilities.
 */

#ifndef MAGI_UTILS_BYTEOPS_H
#define MAGI_UTILS_BYTEOPS_H

#include <stddef.h>
#include <stdint.h>

/** Write one unsigned 8-bit value at offset. */
#define WRITE_U8(buf, off, v) ((buf)[(off)] = (uint8_t)(v))
/** Write one big-endian unsigned 16-bit value at offset. */
#define WRITE_U16(buf, off, v)                                                                     \
  do {                                                                                             \
    (buf)[(off)] = (uint8_t)((v) >> 8);                                                            \
    (buf)[(off) + 1U] = (uint8_t)((v) & 0xFFU);                                                    \
  } while (0)
/** Write one big-endian unsigned 32-bit value at offset. */
#define WRITE_U32(buf, off, v)                                                                     \
  do {                                                                                             \
    (buf)[(off)] = (uint8_t)((v) >> 24);                                                           \
    (buf)[(off) + 1U] = (uint8_t)((v) >> 16);                                                      \
    (buf)[(off) + 2U] = (uint8_t)((v) >> 8);                                                       \
    (buf)[(off) + 3U] = (uint8_t)((v) & 0xFFU);                                                    \
  } while (0)
/** Read one big-endian unsigned 16-bit value from offset. */
#define READ_U16(buf, off) ((uint16_t)((uint16_t)(buf)[(off)] << 8) | (uint16_t)(buf)[(off) + 1U])
/** Read one big-endian unsigned 32-bit value from offset. */
#define READ_U32(buf, off)                                                                         \
  ((uint32_t)((uint32_t)(buf)[(off)] << 24) | (uint32_t)((uint32_t)(buf)[(off) + 1U] << 16) |      \
   (uint32_t)((uint32_t)(buf)[(off) + 2U] << 8) | (uint32_t)(buf)[(off) + 3U])

/**
 * @brief Compute IPv4 header checksum.
 *
 * @param header IPv4 header bytes.
 * @param len Header length in bytes.
 * @return Internet checksum value.
 */
uint16_t ipv4_checksum(const uint8_t* header, size_t len);

/**
 * @brief Compute transport checksum over pseudo header and segment.
 *
 * @param pseudo_hdr Pseudo header bytes.
 * @param ph_len Pseudo header length.
 * @param segment Transport segment bytes.
 * @param seg_len Segment length in bytes.
 * @return Internet checksum value.
 */
uint16_t transport_checksum(const uint8_t* pseudo_hdr, size_t ph_len, const uint8_t* segment,
                            size_t seg_len);

#endif