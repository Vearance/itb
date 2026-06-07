/**
 * @file dhcp.h
 * @brief DHCP client/server over UDP (ports 67/68).
 *
 * Implements the DORA flow: Discover, Offer, Request, Acknowledge.
 * Uses only the MagiSocket API.
 */

#ifndef MAGI_LAYER7_DHCP_H
#define MAGI_LAYER7_DHCP_H

#include "core/node.h"

#define DHCP_SERVER_PORT 67U
#define DHCP_CLIENT_PORT 68U

/* DHCP message types (option 53) */
#define DHCP_DISCOVER 1U
#define DHCP_OFFER 2U
#define DHCP_REQUEST 3U
#define DHCP_ACK 5U

/**
 * @brief Run the full DHCP DORA client flow on a host node.
 *
 * Broadcasts DISCOVER, waits for OFFER, sends REQUEST, waits for ACK,
 * and configures the host's IP address from the yiaddr field.
 *
 * @param node Host node to configure.
 * @return MAGI_OK on success, otherwise an error code.
 */
int dhcp_client_discover(Node* node);

/**
 * @brief Start a DHCP server on a host node.
 *
 * Binds to UDP port 67 and stores the server state in the node.
 * The server will handle incoming DHCP messages via the L4 dispatch.
 *
 * @param node        Host node to run the server on.
 * @param pool_start  First IP in the pool (dotted decimal).
 * @param pool_end    Last IP in the pool (dotted decimal).
 * @param subnet_mask Subnet mask (dotted decimal).
 * @param gateway     Default gateway (dotted decimal).
 * @return MAGI_OK on success, otherwise an error code.
 */
int dhcp_server_start(Node* node, const char* pool_start, const char* pool_end,
                      const char* subnet_mask, const char* gateway);

#endif /* MAGI_LAYER7_DHCP_H */
