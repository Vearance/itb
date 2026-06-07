/**
 * @file l4_host.h
 * @brief L4 integration layer — attaches TCP/UDP handling to a host Node.
 *
 * Sets up node->l4_data as a port registry HashMap and configures
 * node->handle_l4_packet and node->send_ip_packet function pointers.
 */
#ifndef MAGI_LAYER4_L4_HOST_H
#define MAGI_LAYER4_L4_HOST_H

#include "core/node.h"

/**
 * @brief Attach L4 (TCP/UDP) support to a host node.
 *
 * Creates the port registry hashmap, stores it in node->l4_data,
 * and registers the dispatch and send callbacks.
 *
 * @param node  The host node to attach L4 to.
 * @return MAGI_OK on success, MAGI_ERR_NOMEM on allocation failure.
 */
int l4_host_attach(Node* node);

/**
 * @brief Retrieve the port registry from a node's L4 data.
 *
 * @param node  The node whose port registry to retrieve.
 * @return HashMap pointer, or NULL if not attached.
 */
void* l4_host_get_registry(Node* node);

#endif /* MAGI_LAYER4_L4_HOST_H */
