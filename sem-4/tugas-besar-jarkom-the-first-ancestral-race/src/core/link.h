/**
 * @file link.h
 * @brief Link abstraction connecting two interfaces.
 */

#ifndef MAGI_CORE_LINK_H
#define MAGI_CORE_LINK_H

#include <stddef.h>
#include <stdint.h>

struct Interface;

/**
 * @brief Point-to-point link metadata.
 */
typedef struct Link {
  /** First endpoint. */
  struct Interface* endpoint_a;
  /** Second endpoint. */
  struct Interface* endpoint_b;
  /** Simulated propagation delay in milliseconds. */
  uint32_t delay_ms;
  /** Link maximum transmission unit in bytes. */
  uint16_t mtu;
} Link;

/**
 * @brief Create a new link between two interfaces.
 *
 * @param a First endpoint.
 * @param b Second endpoint.
 * @param delay_ms Link delay in milliseconds.
 * @param mtu Link MTU in bytes.
 * @return Link instance, or NULL on failure.
 */
Link* link_new(struct Interface* a, struct Interface* b, uint32_t delay_ms, uint16_t mtu);

/**
 * @brief Destroy a link and detach it from both endpoints.
 *
 * @param link Link to free. NULL is allowed.
 */
void link_free(Link* link);

/**
 * @brief Transmit payload from one endpoint to the opposite endpoint.
 *
 * @param link Link carrying the payload.
 * @param sender Source endpoint on the link.
 * @param data Heap-allocated payload bytes; ownership transfers to the link.
 * @param len Payload length in bytes.
 * @return MAGI_OK on success, otherwise an error code.
 */
int link_transmit(Link* link, struct Interface* sender, const uint8_t* data, size_t len);

#endif
