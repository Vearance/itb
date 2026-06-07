#define _POSIX_C_SOURCE 200809L

#include "byteops.h"

/**
 * @brief Fold carry bits back into the low 16 bits of a checksum accumulator.
 *
 * Repeatedly adds any high 16-bit carry into the low 16 bits until the
 * accumulator fits entirely within 16 bits. This is the standard Internet
 * checksum carry-folding step (RFC 1071).
 *
 * @param sum The 32-bit accumulator from 16-bit word summation.
 * @return Folded 32-bit value with the top 16 bits zeroed.
 */
static uint32_t fold_sum(uint32_t sum) {
  while ((sum >> 16U) != 0U) {
    sum = (sum & 0xFFFFU) + (sum >> 16U);
  }

  return sum;
}

/**
 * @brief Accumulate big-endian 16-bit words from a data buffer into a checksum sum.
 *
 * Processes the data two bytes at a time, interpreting each adjacent pair as
 * a big-endian uint16_t and adding it to the accumulator. A trailing odd byte
 * is zero-padded to 16 bits. Carry is folded after each addition to prevent
 * accumulator overflow.
 *
 * @param sum  Initial accumulator value (typically 0).
 * @param data Pointer to the data buffer.
 * @param len  Length of the data buffer in bytes.
 * @return Accumulated checksum sum (may still contain carries from the
 *         final word — caller should fold or finalize).
 */
static uint32_t accumulate_words(uint32_t sum, const uint8_t* data, size_t len) {
  while (len >= 2U) {
    sum += (uint32_t)(((uint16_t)data[0] << 8) | (uint16_t)data[1]);
    sum = fold_sum(sum);
    data += 2U;
    len -= 2U;
  }

  if (len == 1U) {
    sum += (uint32_t)((uint16_t)data[0] << 8);
    sum = fold_sum(sum);
  }

  return sum;
}

/**
 * @brief Finalize an Internet checksum from a folded accumulator.
 *
 * Performs a final carry fold and then takes the one's complement
 * to produce the 16-bit Internet checksum value ready for wire placement.
 *
 * @param sum The folded accumulator (carries already resolved to low 16 bits).
 * @return The 16-bit one's complement checksum.
 */
static uint16_t finalize_checksum(uint32_t sum) {
  sum = fold_sum(sum);
  return (uint16_t)~sum;
}

uint16_t ipv4_checksum(const uint8_t* header, size_t len) {
  if (header == NULL) {
    return 0U;
  }

  return finalize_checksum(accumulate_words(0U, header, len));
}

uint16_t transport_checksum(const uint8_t* pseudo_hdr, size_t ph_len, const uint8_t* segment,
                            size_t seg_len) {
  uint32_t sum = 0U;

  if (pseudo_hdr != NULL) {
    sum = accumulate_words(sum, pseudo_hdr, ph_len);
  }

  if (segment != NULL) {
    sum = accumulate_words(sum, segment, seg_len);
  }

  return finalize_checksum(sum);
}