#define _POSIX_C_SOURCE 200809L

#include "node.h"

#include "core/interface.h"
#include "core/link.h"
#include "utils/arena.h"
#include "utils/mac.h"
#include "utils/magi_error.h"

#ifdef MAGI_ASYNC
#include "async/queue.h"
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/**
 * @brief Free one interface entry during node teardown.
 */
static void free_interface_entry(const char* key, void* value, void* ctx) {
  (void)key;
  (void)ctx;

  interface_free((Interface*)value);
}

/**
 * @brief Format a numeric port into the node interface map key.
 */
static void build_port_key(uint16_t port, char key[16]) {
  snprintf(key, 16, "%u", (unsigned)port);
}

Node* node_new(const char* name) {
  Node* node = malloc(sizeof(*node));
  if (node == NULL) {
    magi_errno = MAGI_ERR_NOMEM;
    return NULL;
  }

  memset(node, 0, sizeof(*node));
  if (name != NULL) {
    snprintf(node->name, sizeof(node->name), "%s", name);
  }
  node->interfaces = hashmap_new(8U);
  if (node->interfaces == NULL) {
    free(node);
    return NULL;
  }

  node->handle_receive = NULL;
  node->arena = arena_new(ARENA_DEFAULT_CAPACITY);
  if (node->arena == NULL) {
    hashmap_free(node->interfaces);
    free(node);
    return NULL;
  }
#ifdef MAGI_ASYNC
  node->queue = queue_new(256U);
  if (pthread_mutex_init(&node->lock, NULL) != 0) {
    queue_free(node->queue);
    hashmap_free(node->interfaces);
    free(node);
    magi_errno = MAGI_ERR_NOMEM;
    return NULL;
  }
#endif
  return node;
}

void node_free(Node* node) {
  if (node == NULL) {
    return;
  }

  if (node->l4_data_free != NULL) {
    node->l4_data_free(node->l4_data);
  }

  if (node->arena != NULL) {
    arena_free(node->arena);
  }

  if (node->l3_data_free != NULL) {
    node->l3_data_free(node->l3_data);
  }

  if (node->data_free != NULL) {
    node->data_free(node->data);
  }

  if (node->interfaces != NULL) {
    hashmap_foreach(node->interfaces, free_interface_entry, NULL);
    hashmap_free(node->interfaces);
  }

#ifdef MAGI_ASYNC
  queue_free(node->queue);
  pthread_mutex_destroy(&node->lock);
#endif

  free(node);
}

struct Interface* node_add_interface(Node* node, uint16_t port) {
  if (node == NULL || port == 0U) {
    magi_errno = MAGI_ERR_BADARGS;
    return NULL;
  }

  char key[16];
  build_port_key(port, key);

  Interface* existing = hashmap_get(node->interfaces, key);
  if (existing != NULL) {
    return existing;
  }

  Interface* iface = interface_new(node, port);
  if (iface == NULL) {
    return NULL;
  }

  int status = hashmap_set(node->interfaces, key, iface);
  if (status != MAGI_OK) {
    interface_free(iface);
    return NULL;
  }

  return iface;
}

struct Interface* node_get_interface(Node* node, uint16_t port) {
  if (node == NULL || port == 0U) {
    return NULL;
  }

  char key[16];
  build_port_key(port, key);
  return hashmap_get(node->interfaces, key);
}

int node_remove_interface(Node* node, uint16_t port) {
  if (node == NULL || port == 0U) {
    magi_errno = MAGI_ERR_BADARGS;
    return MAGI_ERR_BADARGS;
  }

  char key[16];
  build_port_key(port, key);
  Interface* iface = hashmap_get(node->interfaces, key);

  if (iface == NULL) {
    magi_errno = MAGI_ERR_BADARGS;
    return MAGI_ERR_BADARGS;
  }

  if (iface->link != NULL) {
    magi_errno = MAGI_ERR_BADARGS;
    return MAGI_ERR_BADARGS;
  }

  int status = hashmap_delete(node->interfaces, key);
  interface_free(iface);
  return status;
}
