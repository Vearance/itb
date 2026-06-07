/**
 * @file node.h
 * @brief Common node abstraction used by hosts, switches, and routers.
 */

#ifndef MAGI_CORE_NODE_H
#define MAGI_CORE_NODE_H

#include <stddef.h>
#include <stdint.h>

#include "utils/arena.h"
#include "utils/hashmap.h"

struct Interface;
struct Node;

typedef void (*node_l3_receive_fn)(struct Node* node, struct Interface* iface, const uint8_t* data,
                                   size_t len);
typedef int (*node_l3_send_fn)(struct Node* node, const char* next_hop_ip, uint16_t ethertype,
                               const uint8_t* payload, size_t payload_len);

#ifdef MAGI_ASYNC
#include <pthread.h>

struct MagiQueue;
#endif

/**
 * @brief A generic network node with a set of interfaces.
 */
typedef struct Node {
  /** Stable node name used in topology and logs. */
  char name[64];
  /** Port map keyed by decimal port number string. */
  HashMap* interfaces;
  /** Optional receive handler for frames arriving on an interface. */
  void (*handle_receive)(struct Node* node, struct Interface* iface, const uint8_t* data,
                         size_t len);
  /** Optional layer-specific state owned by this node. */
  void* data;
  /** Optional destructor for layer-specific state. */
  void (*data_free)(void* data);
  /** Optional L3 receive hook used by L2 host code without including layer3 headers. */
  node_l3_receive_fn handle_l3_packet;
  /** Optional L3 send hook used by layer3 host logic without including layer2 headers. */
  node_l3_send_fn send_l3_packet;
  /** Optional L3-owned state. */
  void* l3_data;
  /** Optional destructor for L3-owned state. */
  void (*l3_data_free)(void* data);
  /** Optional arena for hot-path packet allocation. */
  Arena* arena;
  /** Optional L4-specific state (e.g. port registry). */
  void* l4_data;
  /** Optional destructor for L4-specific state. */
  void (*l4_data_free)(void* data);
  /** Optional L4 receive hook called by L3 for non-ICMP protocols. */
  void (*handle_l4_packet)(struct Node* node, const uint8_t src_ip[4], const uint8_t dst_ip[4],
                           uint8_t protocol, const uint8_t* payload, size_t payload_len);
  /** Optional L4→L3 send callback for emitting IP packets. */
  int (*send_ip_packet)(struct Node* node, const uint8_t src_ip[4], const uint8_t dst_ip[4],
                        uint8_t protocol, uint8_t ttl, const uint8_t* data, size_t len);
  /** Host default gateway, if configured. */
  char default_gateway[64];
#ifdef MAGI_ASYNC
  /** Worker thread used in async mode. */
  pthread_t thread;
  /** Async message queue used in async mode. */
  struct MagiQueue* queue;
  /** Node-level lock used in async mode. */
  pthread_mutex_t lock;
#endif
} Node;

/**
 * @brief Allocate and initialize a node.
 *
 * @param name Node name string.
 * @return Node instance, or NULL on failure.
 */
Node* node_new(const char* name);

/**
 * @brief Destroy a node and all owned interfaces.
 *
 * @param node Node to free. NULL is allowed.
 */
void node_free(Node* node);

/**
 * @brief Create or fetch a node interface by port number.
 *
 * @param node Node instance.
 * @param port Port number.
 * @return Interface pointer, or NULL on failure.
 */
struct Interface* node_add_interface(Node* node, uint16_t port);

/**
 * @brief Look up a node interface by port number.
 *
 * @param node Node instance.
 * @param port Port number.
 * @return Matching interface, or NULL if absent.
 */
struct Interface* node_get_interface(Node* node, uint16_t port);

/**
 * @brief Remove an interface from a node.
 *
 * @param node Node instance.
 * @param port Port number.
 * @return MAGI_OK on success, otherwise an error code.
 */
int node_remove_interface(Node* node, uint16_t port);

#endif
