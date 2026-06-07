/**
 * @file interface.h
 * @brief Node interface abstraction and send/receive callbacks.
 */

#ifndef MAGI_CORE_INTERFACE_H
#define MAGI_CORE_INTERFACE_H

#include <stddef.h>
#include <stdint.h>

struct Interface;
struct Link;
struct Node;

#ifdef MAGI_ASYNC
struct MagiQueue;
#endif

/**
 * @brief Callback used by upper layers to send a payload.
 */
typedef void (*send_fn_t)(struct Interface* iface, const uint8_t* data, size_t len);

/**
 * @brief Callback invoked on payload reception.
 */
typedef void (*recv_fn_t)(struct Interface* iface, const uint8_t* data, size_t len);

/**
 * @brief A physical/logical port attached to a node.
 */
typedef struct Interface {
  /** Owning node instance. */
  struct Node* node;
  /** Hardware address of this port. */
  uint8_t mac[6];
  /** Optional CIDR address assigned to this interface. */
  char ip_address[64];
  /** Optional VLAN id for tagged router-facing subinterfaces; 0 means untagged. */
  uint16_t vlan_id;
  /** Port number local to the node. */
  uint16_t port_number;
  /** Attached link, or NULL if disconnected. */
  struct Link* link;
  /** Optional down-stack send hook. */
  send_fn_t send_down;
  /** Optional up-stack receive hook. */
  recv_fn_t receive_up;
#ifdef MAGI_ASYNC
  /** Optional async receive queue in async mode. */
  struct MagiQueue* queue;
#endif
} Interface;

/**
 * @brief Allocate and initialize a new interface for a node.
 *
 * @param node Owning node.
 * @param port Node-local port number.
 * @return Interface instance, or NULL on failure.
 */
Interface* interface_new(struct Node* node, uint16_t port);

/**
 * @brief Destroy an interface instance.
 *
 * @param iface Interface to free. NULL is allowed.
 */
void interface_free(Interface* iface);

/**
 * @brief Transmit data out through the interface's link.
 *
 * @param iface Source interface.
 * @param data Payload bytes.
 * @param len Payload length in bytes.
 * @return MAGI_OK on success, otherwise an error code.
 */
int interface_send(Interface* iface, const uint8_t* data, size_t len);

/**
 * @brief Deliver data received from a link into the owning node.
 *
 * @param iface Destination interface.
 * @param data Heap-allocated payload bytes; ownership transfers to this function.
 * @param len Payload length in bytes.
 */
void interface_receive(Interface* iface, const uint8_t* data, size_t len);

#endif
