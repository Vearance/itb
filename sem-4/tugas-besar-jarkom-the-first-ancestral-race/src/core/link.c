#define _POSIX_C_SOURCE 200809L

#include "link.h"

#include "core/interface.h"
#include "core/node.h"
#include "utils/magi_error.h"

#ifdef MAGI_ASYNC
#include "async/queue.h"
#endif

#include <stdlib.h>
#include <time.h>

/**
 * @brief Simulate link propagation delay by sleeping.
 *
 * Blocks the calling thread for the specified number of milliseconds using
 * nanosleep(). Handles interruption by resuming the sleep for the remaining
 * duration. A zero delay returns immediately.
 *
 * @param delay_ms The delay in milliseconds. 0 means no delay.
 * \return MAGI_OK on success (always succeeds).
 */
static int sleep_for_delay(uint32_t delay_ms) {
  if (delay_ms == 0U) {
    return MAGI_OK;
  }

  struct timespec request;
  request.tv_sec = (time_t)(delay_ms / 1000U);
  request.tv_nsec = (long)((delay_ms % 1000U) * 1000000UL);

  while (nanosleep(&request, &request) != 0) {
    if (request.tv_sec == 0 && request.tv_nsec == 0) {
      break;
    }
  }

  return MAGI_OK;
}

Link* link_new(struct Interface* a, struct Interface* b, uint32_t delay_ms, uint16_t mtu) {
  if (a == NULL || b == NULL) {
    magi_errno = MAGI_ERR_BADARGS;
    return NULL;
  }

  Link* link = malloc(sizeof(*link));
  if (link == NULL) {
    magi_errno = MAGI_ERR_NOMEM;
    return NULL;
  }

  link->endpoint_a = a;
  link->endpoint_b = b;
  link->delay_ms = delay_ms;
  link->mtu = mtu;
  a->link = link;
  b->link = link;
  return link;
}

void link_free(Link* link) {
  if (link == NULL) {
    return;
  }

  if (link->endpoint_a != NULL && link->endpoint_a->link == link) {
    link->endpoint_a->link = NULL;
  }

  if (link->endpoint_b != NULL && link->endpoint_b->link == link) {
    link->endpoint_b->link = NULL;
  }

  free(link);
}

int link_transmit(Link* link, struct Interface* sender, const uint8_t* data, size_t len) {
  if (link == NULL || sender == NULL || data == NULL) {
    magi_errno = MAGI_ERR_BADARGS;
    return MAGI_ERR_BADARGS;
  }

  struct Interface* receiver = NULL;

  if (link->endpoint_a == sender) {
    receiver = link->endpoint_b;
  } else if (link->endpoint_b == sender) {
    receiver = link->endpoint_a;
  } else {
    free((void*)data);
    magi_errno = MAGI_ERR_NOLINK;
    return MAGI_ERR_NOLINK;
  }

  sleep_for_delay(link->delay_ms);

#ifdef MAGI_ASYNC
  if (receiver == NULL || receiver->node == NULL || receiver->node->queue == NULL) {
    free((void*)data);
    magi_errno = MAGI_ERR_BADARGS;
    return MAGI_ERR_BADARGS;
  }

  MagiMsg message;
  message.src_iface = sender;
  message.data = (uint8_t*)data;
  message.len = len;

  int status = queue_push(receiver->node->queue, message);
  if (status != MAGI_OK) {
    free((void*)data);
    magi_errno = status;
    return status;
  }

  return MAGI_OK;
#else
  if (receiver == NULL || receiver->receive_up == NULL) {
    free((void*)data);
    magi_errno = MAGI_ERR_BADARGS;
    return MAGI_ERR_BADARGS;
  }

  receiver->receive_up(receiver, data, len);
  free((void*)data);
  return MAGI_OK;
#endif
}
