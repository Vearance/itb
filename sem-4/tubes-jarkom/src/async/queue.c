#define _POSIX_C_SOURCE 200809L

#include "queue.h"

#include "utils/magi_error.h"

#include <stdlib.h>
#include <string.h>

/**
 * @brief Allocate and initialize a thread-safe bounded queue.
 *
 * Allocates a MagiQueue with a circular buffer of size (capacity + 1)
 * to distinguish full from empty. Initialises the mutex and condition
 * variable. If capacity is zero it is clamped to one.
 *
 * @param capacity Desired queue capacity.
 * @return Pointer to the new queue on success, or NULL with magi_errno
 *         set on allocation or initialisation failure.
 */
MagiQueue* queue_new(size_t capacity) {
  if (capacity == 0U) {
    capacity = 1U;
  }

  MagiQueue* q = calloc(1U, sizeof(*q));
  if (q == NULL) {
    magi_errno = MAGI_ERR_NOMEM;
    return NULL;
  }

  q->cap = capacity + 1U;
  q->buf = calloc(q->cap, sizeof(*q->buf));
  if (q->buf == NULL) {
    free(q);
    magi_errno = MAGI_ERR_NOMEM;
    return NULL;
  }

  if (pthread_mutex_init(&q->lock, NULL) != 0) {
    free(q->buf);
    free(q);
    magi_errno = MAGI_ERR_NOMEM;
    return NULL;
  }

  if (pthread_cond_init(&q->not_empty, NULL) != 0) {
    pthread_mutex_destroy(&q->lock);
    free(q->buf);
    free(q);
    magi_errno = MAGI_ERR_NOMEM;
    return NULL;
  }

  return q;
}

/**
 * @brief Release all resources owned by a queue.
 *
 * Drains any pending messages (freeing their data buffers), then
 * destroys the synchronisation primitives and deallocates the
 * circular buffer and the queue struct itself.
 *
 * @param q Queue to destroy. Passing NULL is safe and results in a
 *          no-op.
 */
void queue_free(MagiQueue* q) {
  if (q == NULL) {
    return;
  }

  while (q->head != q->tail) {
    free(q->buf[q->head].data);
    q->head = (q->head + 1U) % q->cap;
  }

  pthread_cond_destroy(&q->not_empty);
  pthread_mutex_destroy(&q->lock);
  free(q->buf);
  free(q);
}

/**
 * @brief Enqueue one message into the queue.
 *
 * Locks the queue, writes the message at the tail position, advances
 * the tail index, signals the not_empty condition variable, and
 * unlocks. If the queue is full the operation fails immediately
 * without blocking.
 *
 * @param q   Queue handle.
 * @param msg Message to enqueue. The caller transfers ownership of the
 *            embedded data pointer to the queue.
 * @return MAGI_OK on success, MAGI_ERR_BADARGS if q is invalid, or
 *         MAGI_ERR_TIMEOUT if the queue is full.
 */
int queue_push(MagiQueue* q, MagiMsg msg) {
  if (q == NULL || q->buf == NULL) {
    magi_errno = MAGI_ERR_BADARGS;
    return MAGI_ERR_BADARGS;
  }

  if (pthread_mutex_lock(&q->lock) != 0) {
    magi_errno = MAGI_ERR_BADARGS;
    return MAGI_ERR_BADARGS;
  }

  size_t next_tail = (q->tail + 1U) % q->cap;
  if (next_tail == q->head) {
    pthread_mutex_unlock(&q->lock);
    magi_errno = MAGI_ERR_TIMEOUT;
    return MAGI_ERR_TIMEOUT;
  }

  q->buf[q->tail] = msg;
  q->tail = next_tail;
  pthread_cond_signal(&q->not_empty);
  pthread_mutex_unlock(&q->lock);
  return MAGI_OK;
}

/**
 * @brief Dequeue one message from the queue, blocking if empty.
 *
 * Locks the queue and waits on the not_empty condition variable
 * while the buffer is empty. Once a message is available it is
 * copied to out, the head index advances, the slot is zeroed, and
 * the lock is released.
 *
 * @param q   Queue handle.
 * @param out Destination for the dequeued message. The caller assumes
 *            ownership of the data pointer embedded in the message.
 * @return MAGI_OK on success, or MAGI_ERR_BADARGS if any parameter
 *         is NULL or a synchronisation error occurs.
 */
int queue_pop(MagiQueue* q, MagiMsg* out) {
  if (q == NULL || q->buf == NULL || out == NULL) {
    magi_errno = MAGI_ERR_BADARGS;
    return MAGI_ERR_BADARGS;
  }

  if (pthread_mutex_lock(&q->lock) != 0) {
    magi_errno = MAGI_ERR_BADARGS;
    return MAGI_ERR_BADARGS;
  }

  while (q->head == q->tail) {
    if (pthread_cond_wait(&q->not_empty, &q->lock) != 0) {
      pthread_mutex_unlock(&q->lock);
      memset(out, 0, sizeof(*out));
      magi_errno = MAGI_ERR_BADARGS;
      return MAGI_ERR_BADARGS;
    }
  }

  *out = q->buf[q->head];
  memset(&q->buf[q->head], 0, sizeof(q->buf[q->head]));
  q->head = (q->head + 1U) % q->cap;
  pthread_mutex_unlock(&q->lock);

  return MAGI_OK;
}
