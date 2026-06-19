#define _POSIX_C_SOURCE 200809L

#include "engine.h"

#include "utils/magi_error.h"

#ifndef MAGI_ASYNC

int engine_init(struct Topology* topology) {
  (void)topology;
  return MAGI_OK;
}

void engine_shutdown(void) {
}

#else

#include "async/queue.h"
#include "core/interface.h"
#include "core/link.h"
#include "core/node.h"
#include "topology/topology.h"

#include <errno.h>
#include <pthread.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define ENGINE_RIP_INTERVAL_SEC 30L

typedef struct EngineState {
  Topology* topology;
  bool running;
  bool stopping;
  pthread_t timer_thread;
  bool timer_started;
  pthread_mutex_t lock;
  pthread_cond_t stop_cond;
} EngineState;

static EngineState engine_state = {
    .topology = NULL,
    .running = false,
    .stopping = false,
    .timer_thread = 0,
    .timer_started = false,
    .lock = PTHREAD_MUTEX_INITIALIZER,
    .stop_cond = PTHREAD_COND_INITIALIZER,
};

static Interface* message_receiver(Node* node, const MagiMsg* msg) {
  if (node == NULL || msg == NULL || msg->src_iface == NULL || msg->src_iface->link == NULL) {
    return NULL;
  }

  Link* link = msg->src_iface->link;
  Interface* receiver = NULL;
  if (link->endpoint_a == msg->src_iface) {
    receiver = link->endpoint_b;
  } else if (link->endpoint_b == msg->src_iface) {
    receiver = link->endpoint_a;
  }

  return receiver != NULL && receiver->node == node ? receiver : NULL;
}

static void* node_worker(void* ctx) {
  Node* node = (Node*)ctx;
  if (node == NULL || node->queue == NULL) {
    return NULL;
  }

  for (;;) {
    MagiMsg msg = {0};
    if (queue_pop(node->queue, &msg) != MAGI_OK) {
      continue;
    }

    if (msg.data == NULL) {
      break;
    }

    Interface* receiver = message_receiver(node, &msg);
    if (receiver != NULL && receiver->receive_up != NULL) {
      pthread_mutex_lock(&node->lock);
      receiver->receive_up(receiver, msg.data, msg.len);
      pthread_mutex_unlock(&node->lock);
    }

    free(msg.data);
  }

  return NULL;
}

static void run_periodic_ticks(Topology* topology) {
  if (topology == NULL || topology->nodes == NULL) {
    return;
  }

  for (size_t index = 0U; index < topology->nodes->capacity; ++index) {
    HashEntry* entry = &topology->nodes->entries[index];
    if (entry->key == NULL || entry->tombstone) {
      continue;
    }

    TopologyNodeInfo* info = (TopologyNodeInfo*)entry->value;
    Node* node = info != NULL ? info->node : NULL;
    if (node == NULL || node->async_tick_30s == NULL) {
      continue;
    }

    pthread_mutex_lock(&node->lock);
    (void)node->async_tick_30s(node);
    pthread_mutex_unlock(&node->lock);
  }
}

static void* timer_worker(void* ctx) {
  Topology* topology = (Topology*)ctx;

  for (;;) {
    struct timespec deadline;
    if (clock_gettime(CLOCK_REALTIME, &deadline) != 0) {
      return NULL;
    }
    deadline.tv_sec += ENGINE_RIP_INTERVAL_SEC;

    pthread_mutex_lock(&engine_state.lock);
    while (!engine_state.stopping) {
      int wait_status = pthread_cond_timedwait(&engine_state.stop_cond, &engine_state.lock,
                                               &deadline);
      if (wait_status == ETIMEDOUT) {
        break;
      }
    }
    bool stopping = engine_state.stopping;
    pthread_mutex_unlock(&engine_state.lock);

    if (stopping) {
      break;
    }

    run_periodic_ticks(topology);
  }

  return NULL;
}

static int start_node_worker(Node* node) {
  if (node == NULL || node->queue == NULL || node->async_worker_started) {
    return MAGI_OK;
  }

  if (pthread_create(&node->thread, NULL, node_worker, node) != 0) {
    magi_errno = MAGI_ERR_NOMEM;
    return MAGI_ERR_NOMEM;
  }

  node->async_worker_started = true;
  return MAGI_OK;
}

static void enqueue_stop(Node* node) {
  if (node == NULL || node->queue == NULL || !node->async_worker_started) {
    return;
  }

  MagiMsg sentinel = {0};
  (void)queue_push_blocking(node->queue, sentinel);
}

static void join_node_worker(Node* node) {
  if (node == NULL || !node->async_worker_started) {
    return;
  }

  pthread_join(node->thread, NULL);
  node->async_worker_started = false;
}

static void foreach_node(Topology* topology, void (*fn)(Node* node)) {
  if (topology == NULL || topology->nodes == NULL || fn == NULL) {
    return;
  }

  for (size_t index = 0U; index < topology->nodes->capacity; ++index) {
    HashEntry* entry = &topology->nodes->entries[index];
    if (entry->key == NULL || entry->tombstone) {
      continue;
    }

    TopologyNodeInfo* info = (TopologyNodeInfo*)entry->value;
    if (info != NULL) {
      fn(info->node);
    }
  }
}

int engine_init(Topology* topology) {
  if (topology == NULL) {
    magi_errno = MAGI_ERR_BADARGS;
    return MAGI_ERR_BADARGS;
  }

  pthread_mutex_lock(&engine_state.lock);
  if (engine_state.running) {
    pthread_mutex_unlock(&engine_state.lock);
    return MAGI_OK;
  }

  engine_state.topology = topology;
  engine_state.stopping = false;
  engine_state.running = true;
  pthread_mutex_unlock(&engine_state.lock);

  if (topology->nodes != NULL) {
    for (size_t index = 0U; index < topology->nodes->capacity; ++index) {
      HashEntry* entry = &topology->nodes->entries[index];
      if (entry->key == NULL || entry->tombstone) {
        continue;
      }

      TopologyNodeInfo* info = (TopologyNodeInfo*)entry->value;
      int status = start_node_worker(info != NULL ? info->node : NULL);
      if (status != MAGI_OK) {
        engine_shutdown();
        return status;
      }
    }
  }

  if (pthread_create(&engine_state.timer_thread, NULL, timer_worker, topology) != 0) {
    engine_shutdown();
    magi_errno = MAGI_ERR_NOMEM;
    return MAGI_ERR_NOMEM;
  }
  engine_state.timer_started = true;

  return MAGI_OK;
}

void engine_shutdown(void) {
  pthread_mutex_lock(&engine_state.lock);
  if (!engine_state.running) {
    pthread_mutex_unlock(&engine_state.lock);
    return;
  }

  Topology* topology = engine_state.topology;
  engine_state.stopping = true;
  pthread_cond_broadcast(&engine_state.stop_cond);
  pthread_mutex_unlock(&engine_state.lock);

  foreach_node(topology, enqueue_stop);
  foreach_node(topology, join_node_worker);

  if (engine_state.timer_started) {
    pthread_join(engine_state.timer_thread, NULL);
    engine_state.timer_started = false;
  }

  pthread_mutex_lock(&engine_state.lock);
  engine_state.topology = NULL;
  engine_state.running = false;
  engine_state.stopping = false;
  pthread_mutex_unlock(&engine_state.lock);
}

#endif
