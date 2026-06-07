/**
 * @file queue.h
 * @brief Async queue primitives used when MAGI_ASYNC is enabled.
 */

#ifndef MAGI_ASYNC_QUEUE_H
#define MAGI_ASYNC_QUEUE_H

#include <pthread.h>
#include <stddef.h>
#include <stdint.h>

struct Interface;

/**
 * @brief A queued frame payload for async delivery.
 */
typedef struct MagiMsg {
  /** Source interface that produced the payload. */
  struct Interface* src_iface;
  /** Raw payload buffer. Receiver owns and frees after dequeue. */
  uint8_t* data;
  /** Payload length in bytes. */
  size_t len;
} MagiMsg;

/**
 * @brief Single-producer/single-consumer queue state.
 */
typedef struct MagiQueue {
  /** Circular buffer storage for queue items. */
  MagiMsg* buf;
  /** Read index in the circular buffer. */
  size_t head;
  /** Write index in the circular buffer. */
  size_t tail;
  /** Maximum number of queue entries. */
  size_t cap;
  /** Mutex protecting queue state. */
  pthread_mutex_t lock;
  /** Condition signaled when queue transitions from empty. */
  pthread_cond_t not_empty;
} MagiQueue;

/**
 * @brief Allocate and initialize a queue.
 *
 * @param capacity Desired queue capacity.
 * @return Queue instance, or NULL on allocation/initialization failure.
 */
MagiQueue* queue_new(size_t capacity);

/**
 * @brief Release all resources owned by a queue.
 *
 * @param q Queue to destroy. NULL is allowed.
 */
void queue_free(MagiQueue* q);

/**
 * @brief Push one message into the queue.
 *
 * @param q Queue handle.
 * @param msg Message to enqueue.
 * @return MAGI_OK on success, otherwise an error code.
 */
int queue_push(MagiQueue* q, MagiMsg msg);

/**
 * @brief Pop one message from the queue.
 *
 * @param q Queue handle.
 * @param out Destination for the popped message.
 * @return MAGI_OK on success, otherwise an error code.
 */
int queue_pop(MagiQueue* q, MagiMsg* out);

#endif
